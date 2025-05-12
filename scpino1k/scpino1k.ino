#define USE_TIMER_1 true

#include "libs/TimerInterrupt.h"

#include "stdint_aliases.h"

#define RBI "rbi-scpino1k"
#define VER "0.1"

#define ADC_PIN A0

#define STREAM_INTERVAL_MILIS 1000
#define STREAM_BUFFER_SIZE 50

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

bool stream_data = false;
bool send_stream_data = false;
struct {
    u16 buffer1[STREAM_BUFFER_SIZE];
    u16 buffer2[STREAM_BUFFER_SIZE];
    u8  current_write_buffer;

    u16* get_write_buffer() {
        return this->current_write_buffer == 0
            ? this->buffer1
            : this->buffer2
        ;
    }

    u16* get_read_buffer() {
        return this->current_write_buffer == 0
            ? this->buffer2
            : this->buffer1
        ;
    }

    void flip_buffers() {
        this->current_write_buffer ^= 1;
    }
} stream_buffer;


// SCPI-like command handling
void processCommand(String cmd) {
    if (cmd == "*IDN?") { // identification (standard SCPI command)
        Serial.println(RBI);
    }
    else if (cmd == "*VER?") { // version number (*not* a standard SCPI command)
        Serial.println(VER);
    }

    else if (cmd.startsWith(":MEAS")) {
        Serial.println(analogRead(ADC_PIN));
    }
    else if (cmd.startsWith(":BRST")) {
        for (int i = 0; i < 50; i++) {
            stream_buffer.buffer1[i] = analogRead(ADC_PIN);
            delayMicroseconds(5);
        }

        for (int i = 0; i < 50; i++) {
            Serial.println(stream_buffer.buffer1[i]);
        }
    }
    else if (cmd.startsWith(":STRM")) {
        // stream_data = !cmd.startsWith(":STRM:STOP");
        if (cmd.startsWith(":STRM:STOP")) {
            ITimer1.stopTimer();
            digitalWrite(LED_BUILTIN, LOW);
        }
        else {
            ITimer1.restartTimer();
            digitalWrite(LED_BUILTIN, HIGH);
        }
    }
    else {
        Serial.print("ERR -- unsupported command: ");
        Serial.println(cmd);
    }
}


void TimerHandler1() {
    static u16 measureCount = 0;
    stream_buffer.get_write_buffer()[measureCount++] = analogRead(ADC_PIN);

    if (measureCount == STREAM_BUFFER_SIZE) {
        measureCount = 0;
        stream_buffer.flip_buffers();
        send_stream_data = true;
    }
}

void setup() {
    SerialInputStr.reserve(256);
    Serial.begin(115200);
    while (!Serial);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(ADC_PIN, INPUT);

    ITimer1.init();
    if (!ITimer1.attachInterruptInterval(STREAM_INTERVAL_MILIS / STREAM_BUFFER_SIZE, TimerHandler1)) {
        Serial.println("Couldn’t attach interrupt timer 1");
        for (;;);
    }
    ITimer1.stopTimer();
}


void loop() {
    if (send_stream_data) {
        send_stream_data = false;
        u16* buffer = stream_buffer.get_read_buffer();

        for (size_t i = 0; i < STREAM_BUFFER_SIZE; i++) {
            Serial.print("    |");
            u8 graph_star_position = buffer[i] * 20 / 1024;
            for (size_t j = 0; j < graph_star_position; j++) {
                Serial.print(" ");
            }
            Serial.print("*");
            for (size_t j = 0; j < 20 - graph_star_position; j++) {
                Serial.print(" ");
            }
            Serial.print("|    ");

            Serial.println(buffer[i]);

        }
    }

    while (Serial.available()) {
        char inputChr = (char) Serial.read();
        if (inputChr == '\n') {
            processCommand(SerialInputStr);
            SerialInputStr = "";
        } else {
            SerialInputStr += inputChr;
        }
    }
}
