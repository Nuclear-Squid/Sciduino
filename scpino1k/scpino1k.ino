#define USE_TIMER_1 true

#include "libs/TimerInterrupt.h"

#include "flip_flop.h"
#include "stdint_aliases.h"

#define RBI "rbi-scpino1k"
#define VER "0.2"

#define ADC_PIN A0

#define STREAM_BUFFER_SIZE 50
#define STREAM_FREQUENCY STREAM_BUFFER_SIZE

#define BURST_BUFFER_SIZE 500
#define BURST_FREQUENCY 5000

// Pins :
//
// 11 -> DOUT = Serial Data Output. When _CS_ is low, data is clocked out of
//       DOUT with each falling SCLK transition. When _CS_ is high, DOUT is high
//       impedance.
//
// 12 -> DIN = Serial Data Input. When _CS_ is low, data is clocked in on the
//       rising edge of SCLK. When _CS_ is high, transitions on DIN are ignored.
//
// 13 -> SCLK = Serial Clock Input. When _CS_ is low, transitions on SCLK clock
//       data into DIN and out of DOUT. When _CS_ is high, transitions on SCLK
//       are ignored. high, transitions on DIN are ignored
//
// NOTE: _CS_ is currently connected to GND, thus always low.

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

// SCPI-like command handling
void processCommand(String cmd) {
    if (cmd == "*IDN?") { // identification (standard SCPI command)
        Serial.println(RBI);
        return;
    }
    if (cmd == "*VER?") { // version number (*not* a standard SCPI command)
        Serial.println(VER);
        return;
    }

    if (cmd.startsWith(":MEAS")) {
        Serial.println(analogRead(ADC_PIN));
        return;
    }

    if (cmd.startsWith(":BRST")) {
        measure_count = 0;
        if (!ITimer1.attachInterrupt(BURST_FREQUENCY, TimerHandlerBurst)) {
            Serial.println("Couldn’t attach interrupt timer 1");
        }
        return;
    }

    if (cmd.startsWith(":STRM")) {
        measure_count = 0;
        if (cmd.startsWith(":STRM:STOP")) {
            ITimer1.stopTimer();
        }
        else if (!ITimer1.attachInterrupt(STREAM_FREQUENCY, TimerHandlerStream)) {
            Serial.println("Couldn’t attach interrupt timer 1");
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

    if (measure_count == BURST_BUFFER_SIZE) {
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
            for (size_t i = 0; i < BURST_BUFFER_SIZE; i++) Serial.println(buffers.burst[i]);
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
