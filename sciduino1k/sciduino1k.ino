#define USE_TIMER_3 true // NOTE: Needed by `TimerInterrupt`, keep above include statements

#include "3rd_parts/TimerInterrupt.h"

#include "sciduino1k.h"
#include "flip_flop.h"
#include "stdint_aliases.h"

#define RBI "rbi-sciduino1k"
#define VER "0.4"

#define STREAM_BUFFER_SIZE 100
#define STREAM_FREQUENCY STREAM_BUFFER_SIZE

#define BURST_BUFFER_SIZE 1000
#define BURST_FREQUENCY 5000

String SerialInputStr = "";

WaveformHeader waveform_header;
WaveformHeader waveform_header2;

enum: u8 {
    ReadCommands,
    SendBurstData,
    SendStreamData,
} serial_state = ReadCommands;

enum: u8 {
    Ascii,
    Binary,
} transmission_format = Binary;

union {
    FlipFlopBuffer<u16, STREAM_BUFFER_SIZE> stream;
    struct {
        u16 burst[BURST_BUFFER_SIZE];
        u16 burst2[BURST_BUFFER_SIZE];
    };
} buffers;

u32 measure_count = 0;
u32 max_measurements = 0;

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

    if (try_pop_command(&cmd, ":sources:get", ":sources:get")) {
        const size_t ai_size = sizeof(AnalogInput);
        const size_t analog_inputs_count = sizeof(analog_inputs) / ai_size;
        Serial.write(analog_inputs_count);
        for (auto i = 0; i < analog_inputs_count; i++) {
            char buffer[ai_size];
            Serial.write((char*) memcpy_P(buffer, &analog_inputs[i], ai_size), ai_size);
        }
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

        Serial.print(F("Err -- Unknown format: "));
        Serial.print(cmd);
        return;
    }

    if (try_pop_command(&cmd, ":measure", ":meas")) {
        Serial.println(analogRead(analog_inputs[0].get_pin()));
        return;
    }

    if (try_pop_command(&cmd, ":burst", ":bur")) {
        u8 comma_index = cmd.indexOf(',');
        if (comma_index == -1 || comma_index != cmd.lastIndexOf(',')) {
            Serial.println(F("Err -- :BURST command takes two arguments: meas,freq"));
            return;
        }

        i16 measurements = cmd.substring(0, comma_index).toInt();
        if (measurements <= 0) {
            Serial.println(F("Err -- Number of measurements should be a strictly positive integer"));
            return;
        }

        if (measurements > BURST_BUFFER_SIZE) {
            Serial.print(F("Err -- Number of measurements exceds maximum allowed: "));
            Serial.println(BURST_BUFFER_SIZE);
            return;
        }

        float frequency = cmd.substring(comma_index + 1).toFloat();
        if (frequency <= 0) {
            Serial.println(F("Err -- Invalid frequency (Hz) requested, should be a strictly positive float"));
            return;
        }

        measure_count = 0;
        waveform_header = WaveformHeader {
            .initial_time = 0,
            .time_interval = 1 / frequency,
            .values_count = measurements,
            .pin = analog_inputs[0].get_pin(),
        };

        waveform_header2 = WaveformHeader {
            .initial_time = 0,
            .time_interval = 1 / frequency,
            .values_count = measurements,
            .pin = analog_inputs[1].get_pin(),
        };

        if (!ITimer3.attachInterrupt(frequency, TimerHandlerBurst)) {
            Serial.println(F("Err -- Couldn’t attach interrupt timer 1"));
        }
        return;
    }

    if (try_pop_command(&cmd, ":stream", ":str")) {
        if (cmd.startsWith(":stop")) {
            ITimer3.stopTimer();
            return;
        }

        if (cmd.indexOf(',') != -1 || cmd == "") {
            Serial.println(F("Err -- STREAM command accepts exactly 1 argument: frequency"));
            return;
        }

        float frequency = cmd.toFloat();
        if (frequency <= 0) {
            Serial.println(F("Err -- Invalid frequency (Hz) requested, should be a strictly positive float"));
            return;
        }

        measure_count = 0;
        if (!ITimer3.attachInterrupt(frequency, TimerHandlerStream)) {
            Serial.println(F("Err -- Couldn’t attach interrupt timer 1"));
        }

        return;
    }

    Serial.print(F("ERR -- unsupported command: "));
    Serial.println(cmd);
}


void TimerHandlerStream() {
    buffers.stream.get_write_buffer()[measure_count++] = analogRead(analog_inputs[0].get_pin());

    if (measure_count == buffers.stream.length()) {
        measure_count = 0;
        buffers.stream.flip_buffers();
        serial_state = SendStreamData;
    }
}


void TimerHandlerBurst() {
    buffers.burst[measure_count]  = analogRead(analog_inputs[0].get_pin());
    buffers.burst2[measure_count] = analogRead(analog_inputs[1].get_pin());
    measure_count++;

    if (measure_count == waveform_header.values_count) {
        ITimer3.stopTimer();
        measure_count = 0;
        serial_state = SendBurstData;
    }
}


void setup() {
    SerialInputStr.reserve(256);
    Serial.begin(115200);
    while (!Serial);

    const size_t analog_inputs_count = sizeof(analog_inputs) / sizeof(AnalogInput);
    for (auto i = 0; i < analog_inputs_count; i++)
        pinMode(analog_inputs[i].get_pin(), INPUT);

    pinMode(LED_BUILTIN, OUTPUT);

    ITimer3.init();
}


void loop() {
    switch (serial_state) {
        case ReadCommands: {
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
        }

        case SendBurstData: {
            if (transmission_format == Ascii) {
                for (size_t i = 0; i < max_measurements; i++) Serial.println(buffers.burst[i]);
                serial_state = ReadCommands;
                break;
            }

            if (transmission_format == Binary) {
                // We need to send a u16 array as bytes via the serial bus, but
                // Serial.write only allows sending buffers of type const char*.
                // This is one of the very few cases where `reinterpret_cast` is
                // **actually needed**, yet using it still feels wrong
                Serial.write(reinterpret_cast<const char*>(&waveform_header), sizeof(WaveformHeader));
                Serial.write(
                    reinterpret_cast<const char*>(buffers.burst),
                    waveform_header.values_count * sizeof(u16)
                );
                Serial.write(reinterpret_cast<const char*>(&waveform_header2), sizeof(WaveformHeader));
                Serial.write(
                    reinterpret_cast<const char*>(buffers.burst2),
                    waveform_header2.values_count * sizeof(u16)
                );
                serial_state = ReadCommands;
                break;
            }
        }

        case SendStreamData: {
            if (transmission_format == Ascii) {
                u16* values = buffers.stream.get_read_buffer();
                for (size_t i = 0; i < buffers.stream.length(); i++) Serial.println(values[i]);
                serial_state = ReadCommands;
                break;
            }

            if (transmission_format == Binary) {
                // Same thing here, we **actually need** reinterpret_cast...
                Serial.write(
                    reinterpret_cast<const char*>(buffers.stream.get_read_buffer()),
                    buffers.stream.length() * sizeof(u16)
                );
                serial_state = ReadCommands;
                break;
            }
        }
    }
}
