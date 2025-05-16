#define USE_TIMER_1 true // NOTE: Needed by `TimerInterrupt`, keep above include statements

#include "3rd_parts/TimerInterrupt.h"

#include "flip_flop.h"
#include "stdint_aliases.h"

#define RBI "rbi-scpino1k"
#define VER "0.3"

#define ADC_PIN A0

#define STREAM_BUFFER_SIZE 1000
#define STREAM_FREQUENCY STREAM_BUFFER_SIZE

#define BURST_BUFFER_SIZE 2000
#define BURST_FREQUENCY 5000

String SerialInputStr = "";

// XXX: The two following enums should never overlap bit-wise, as they are
// combined with a logical or later to more easilly swicth over both of them
// TODO: Find a better way to represent a "switch-able" format for the
// format / state pair.
enum: u8 {
    ReadCommands   = 1,
    SendBurstData  = 1 << 1,
    SendStreamData = 1 << 2,
} serial_state = ReadCommands;

enum: u8 {
    Ascii  = 1 << 3,
    Binary = 1 << 4,
} transmission_format = Binary;

union {
    FlipFlopBuffer<u16, STREAM_BUFFER_SIZE> stream;
    u16 burst[BURST_BUFFER_SIZE];
} buffers;

size_t measure_count = 0;
size_t max_measurements = 0;


bool try_pop_command(String* str, const String& long_command, const String& short_command) {
    const String* matching_prefix = str->startsWith(long_command)
        ? &long_command
        : str->startsWith(short_command)
        ? &short_command
        : nullptr
    ;

    if (!matching_prefix) return false;
    str->remove(0, matching_prefix->length());
    str->trim();
    return true;
}

// SCPI-like command handling
void processCommand(String cmd) {
    cmd.trim();
    cmd.toLowerCase();

    if (cmd == "*idn?") { // identification (standard SCPI command)
        Serial.println(RBI);
        return;
    }
    if (cmd == "*ver?") { // version number (*not* a standard SCPI command)
        Serial.println(VER);
        return;
    }

    if (try_pop_command(&cmd, ":format", ":for")) {
        if (cmd == "?") {
            Serial.println(transmission_format == Ascii ? 'A' : 'B');
            return;
        }

        if (try_pop_command(&cmd, ":ascii", ":a")) {
            transmission_format = Ascii;
            return;
        }

        if (try_pop_command(&cmd, ":binary", ":b")) {
            transmission_format = Binary;
            return;
        }

        Serial.print("Err -- Unknown format: ");
        Serial.print(cmd);
        return;
    }

    if (try_pop_command(&cmd, ":measure", ":meas")) {
        Serial.println(analogRead(ADC_PIN));
        return;
    }

    if (try_pop_command(&cmd, ":burst", ":bur")) {
        u8 comma_index = cmd.indexOf(',');
        if (comma_index == -1 || comma_index != cmd.lastIndexOf(',')) {
            Serial.println("Err -- :BURST command takes two arguments: meas,freq");
            return;
        }

        i16 measurements = cmd.substring(0, comma_index).toInt();
        if (measurements <= 0) {
            Serial.println("Err -- Number of measurements should be a strictly positive integer");
            return;
        }

        if (measurements > BURST_BUFFER_SIZE) {
            Serial.print("Err -- Number of measurements exceds maximum allowed: ");
            Serial.println(BURST_BUFFER_SIZE);
            return;
        }

        float frequency = cmd.substring(comma_index + 1).toFloat();
        if (frequency <= 0) {
            Serial.println("Err -- Invalid frequency (Hz) requested, should be a strictly positive float");
            return;
        }

        measure_count = 0;
        max_measurements = measurements;
        if (!ITimer1.attachInterrupt(frequency, TimerHandlerBurst)) {
            Serial.println("Err -- Couldn’t attach interrupt timer 1");
        }
        return;
    }

    if (try_pop_command(&cmd, ":stream", ":str")) {
        if (cmd.startsWith(":stop")) {
            ITimer1.stopTimer();
            return;
        }

        if (cmd.indexOf(',') != -1 || cmd == "") {
            Serial.println("Err -- STREAM command accepts exactly 1 argument: frequency");
            return;
        }

        float frequency = cmd.toFloat();
        if (frequency <= 0) {
            Serial.println("Err -- Invalid frequency (Hz) requested, should be a strictly positive float");
            return;
        }

        measure_count = 0;
        if (!ITimer1.attachInterrupt(frequency, TimerHandlerStream)) {
            Serial.println("Err -- Couldn’t attach interrupt timer 1");
        }

        return;
    }

    Serial.print("ERR -- unsupported command: ");
    Serial.println(cmd);
}


void TimerHandlerStream() {
    buffers.stream.get_write_buffer()[measure_count++] = analogRead(ADC_PIN);

    if (measure_count == buffers.stream.length()) {
        measure_count = 0;
        buffers.stream.flip_buffers();
        serial_state = SendStreamData;
    }
}


void TimerHandlerBurst() {
    buffers.burst[measure_count++] = analogRead(ADC_PIN);

    if (measure_count == max_measurements) {
        ITimer1.stopTimer();
        measure_count = 0;
        serial_state = SendBurstData;
    }
}


void setup() {
    SerialInputStr.reserve(256);
    Serial.begin(115200);
    while (!Serial);

    pinMode(ADC_PIN, INPUT);

    ITimer1.init();
}


void loop() {
    switch (serial_state | transmission_format) {
        case ReadCommands | Ascii:
        case ReadCommands | Binary:
            while (Serial.available()) {
                char inputChr = (char) Serial.read();
                if (inputChr == '\n') {
                    processCommand(SerialInputStr);
                    SerialInputStr = "";
                } else {
                    SerialInputStr += inputChr;
                }
            }
            break;

        case SendBurstData | Ascii:
            for (size_t i = 0; i < max_measurements; i++) Serial.println(buffers.burst[i]);
            serial_state = ReadCommands;
            break;

        case SendBurstData | Binary:
            // We need to send a u16 array as bytes via the serial bus, but
            // Serial.write only allows sending buffers of type const char*.
            // This is one of the very few cases where `reinterpret_cast` is
            // **actually needed**, yet using it still feels wrong
            Serial.write(
                reinterpret_cast<const char*>(buffers.burst),
                max_measurements * sizeof(u16)
            );
            serial_state = ReadCommands;
            break;

        case SendStreamData | Ascii:
            u16* values = buffers.stream.get_read_buffer();
            for (size_t i = 0; i < buffers.stream.length(); i++) Serial.println(values[i]);
            serial_state = ReadCommands;
            break;

        case SendStreamData | Binary:
            // Same thing here, we **actually need** reinterpret_cast...
            Serial.write(
                reinterpret_cast<const char*>(buffers.stream.get_read_buffer()),
                buffers.stream.length() * sizeof(u16)
            );
            serial_state = ReadCommands;
            break;
    }
}
