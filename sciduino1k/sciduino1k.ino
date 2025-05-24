#define USE_TIMER_1 true // NOTE: Needed by `TimerInterrupt`, keep above include statements

// #include "3rd_parts/TimerInterrupt.h"
#include <DueTimer.h>

#include "sciduino1k.h"
#include "stdint_aliases.h"
#include "waveforms.h"

#define RBI "rbi-sciduino1k"
#define VER "0.5"


String serial_input = "";

struct WaveformArray<ANALOG_INPUT_COUNT, WAVEFORM_BUFFER_BYTE_SIZE> waveforms;

TransmissionFormat transmission_format = TransmissionFormat::Binary;

bool try_pop_command(String* str, const String& long_command, const String& short_command) {
    if (str->startsWith(long_command)) {
        str->remove(0, long_command.length());
        str->trim();
        return true;
    }

    if (str->startsWith(short_command)) {
        str->remove(0, short_command.length());
        str->trim();
        return true;
    }

    return false;
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

    if (try_pop_command(&cmd, ":inputs?", ":in?")) {
        if (transmission_format == TransmissionFormat::Binary) {
            Serial.write(ANALOG_INPUT_COUNT);
            for (auto i = 0; i < ANALOG_INPUT_COUNT; i++) {
                const size_t input_size = sizeof(AnalogInput);
                char buffer[input_size];
                Serial.write((char*) memcpy_P(buffer, &analog_inputs[i], input_size), input_size);
            }
        }
        else {
            char scientific_float_buffer[16];
            Serial.print(F(":input:"));
            for (auto i = 0; i < ANALOG_INPUT_COUNT; i++) {
                Serial.print(F("begin"));
                Serial.print(F(";name "));
                Serial.print(analog_inputs[i].name);
                Serial.print(F(";unit "));
                Serial.print(analog_inputs[i].unit);

                Serial.print(F(";gain "));
                sprintf(scientific_float_buffer, "%.6e", analog_inputs[i].gain);
                Serial.print(scientific_float_buffer);

                Serial.print(F(";offset "));
                sprintf(scientific_float_buffer, "%.6e", analog_inputs[i].offset);
                Serial.print(scientific_float_buffer);

                Serial.print(F(";precision "));
                Serial.print(analog_inputs[i].precision);
                Serial.print(F(";pin "));
                Serial.print(analog_inputs[i].pin);
                Serial.print(F( ";end" ));
                Serial.print(i == ANALOG_INPUT_COUNT - 1 ? '\n' : ';');
            }
        }
        return;
    }

    if (try_pop_command(&cmd, ":format", ":for")) {
        if (cmd == "?") {
            Serial.println(transmission_format == TransmissionFormat::Ascii ? 'A' : 'B');
            return;
        }

        if (try_pop_command(&cmd, ":ascii", ":a")) {
            transmission_format = TransmissionFormat::Ascii;
            return;
        }

        if (try_pop_command(&cmd, ":binary", ":b")) {
            transmission_format = TransmissionFormat::Binary;
            return;
        }

        Serial.print(F("Err -- Unknown format: "));
        Serial.print(cmd);
        return;
    }

    if (try_pop_command(&cmd, ":measure", ":meas")) {
        Serial.println(analogRead(analog_inputs[0].pin));
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

        float frequency = cmd.substring(comma_index + 1).toFloat();
        if (frequency <= 0) {
            Serial.println(F("Err -- Invalid frequency (Hz) requested, should be a strictly positive float"));
            return;
        }

        WaveformHeader header = {
            .length = measurements,
            .time = 0,
            .interval = 1 / frequency,
            .pin = 0,  // Default value, depends on the analog input
        };

        waveforms.clear();
        for (size_t i = 0; i < ANALOG_INPUT_COUNT; i++) {
            header.pin = analog_inputs[i].pin;
            if (!waveforms.add_waveform(header)) {
                Serial.println("Err -- could not allocate enough memory for waveforms");
                waveforms.clear();
                return;
            }
        }

        Timer1.attachInterrupt(timer_handler_burst).setFrequency(frequency).start();
        return;
    }

    if (try_pop_command(&cmd, ":stream", ":str")) {
        if (cmd.startsWith(":stop")) {
            Timer1.stop();
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

        WaveformHeader header = {
            .length = 50,
            .time = 0,
            .interval = 1 / frequency,
            .pin = 0,  // Default value, depends on the analog input
        };

        waveforms.clear();
        for (size_t i = 0; i < ANALOG_INPUT_COUNT; i++) {
            header.pin = analog_inputs[i].pin;
            if (!waveforms.add_waveform(header)) {
                Serial.println("Err -- could not allocate enough memory for waveforms");
                waveforms.clear();
                return;
            }
        }

        Timer1.attachInterrupt(timer_handler_stream).setFrequency(frequency).start();

        return;
    }

    Serial.print(F("ERR -- unsupported command: "));
    Serial.println(cmd);
}


void timer_handler_stream() {
    FillStatus status;  // All waveforms are the same length and are in sync
    for (size_t i = 0; i < waveforms.active_count; i++)
        status = waveforms.arr[i].push(analogRead(waveforms.arr[i].meta.pin));

    switch (status) {
        case FillStatus::DontWorry: break;

        case FillStatus::HalfFull:
            waveforms.schedule_transmission(transmission_format, BufferSubset::FirstHalf);
            break;

        case FillStatus::CompletellyFull:
            waveforms.schedule_transmission(transmission_format, BufferSubset::SecondHalf);
            break;
    }
}


void timer_handler_burst() {
    FillStatus status;  // All waveforms are the same length and are in sync
    for (size_t i = 0; i < waveforms.active_count; i++) {
        status = waveforms.arr[i].push(analogRead(waveforms.arr[i].meta.pin));
    }

    if (status == FillStatus::CompletellyFull) {
        Timer1.stop();
        waveforms.schedule_transmission(transmission_format, BufferSubset::Full);
    }
}


void setup() {
    serial_input.reserve(256);
    Serial.begin(115200);

    for (auto i = 0; i < ANALOG_INPUT_COUNT; i++)
        pinMode(analog_inputs[i].pin, INPUT);

    while (!Serial);
}


void loop() {
    waveforms.process_scheduled_transmission();

    while (Serial.available()) {
        char input = (char) Serial.read();
        if (input == '\n') {
            processCommand(serial_input);
            serial_input = "";
        } else {
            serial_input += input;
        }
    }
}
