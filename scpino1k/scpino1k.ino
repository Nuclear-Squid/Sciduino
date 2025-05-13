#define USE_TIMER_1 true // NOTE: Needed by `TimerInterrupt`, keep above include statements

#include "libs/TimerInterrupt.h"

#include "flip_flop.h"
#include "stdint_aliases.h"

#define RBI "rbi-scpino1k"
#define VER "0.3"

#define ADC_PIN A0

#define STREAM_BUFFER_SIZE 500
#define STREAM_FREQUENCY STREAM_BUFFER_SIZE

#define BURST_BUFFER_SIZE 2000
#define BURST_FREQUENCY 5000

String SerialInputStr = "";

enum: u8 {
    ReadCommands,
    SendBurstData,
    SendStreamData,
} serial_state;

union {
    FlipFlopBuffer<u16, STREAM_BUFFER_SIZE> stream;
    u16 burst[BURST_BUFFER_SIZE];
} buffers;

size_t measure_count = 0;
size_t max_measurements = 0;

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

    if (cmd.startsWith(":meas")) {
        Serial.println(analogRead(ADC_PIN));
        return;
    }

    if (cmd.startsWith(":brst")) {
        cmd.remove(0, 5);
        cmd.trim();

        u8 comma_index = cmd.indexOf(',');
        if (comma_index == -1 || comma_index != cmd.lastIndexOf(',')) {
            Serial.println("Err -- BRST command takes two arguments: meas,freq");
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

    if (cmd.startsWith(":strm")) {
        measure_count = 0;
        if (cmd.startsWith(":strm:stop")) {
            ITimer1.stopTimer();
        }
        else if (!ITimer1.attachInterrupt(STREAM_FREQUENCY, TimerHandlerStream)) {
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
    switch (serial_state) {
        case ReadCommands:
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

        case SendBurstData:
            for (size_t i = 0; i < max_measurements; i++) Serial.println(buffers.burst[i]);
            serial_state = ReadCommands;
            break;

        case SendStreamData:
            u16* buffer = buffers.stream.get_read_buffer();

            for (size_t i = 0; i < STREAM_BUFFER_SIZE; i++) {
                Serial.print("    |");
                size_t graph_star_position = buffer[i] * 20 / 1024;
                for (u8 j = 0; j < graph_star_position; j++) {
                    Serial.print(" ");
                }
                Serial.print("*");
                for (u8 j = 0; j < 20 - graph_star_position; j++) {
                    Serial.print(" ");
                }
                Serial.print("|    ");

                Serial.println(buffer[i]);
            }
            serial_state = ReadCommands;
            break;
    }
}
