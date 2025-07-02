#define USE_TIMER_1 true // NOTE: Needed by `TimerInterrupt`, keep above include statements

#include "adc.h"
#include "sciduino1k.h"
#include "stdint_aliases.h"
#include "timers.h"
#include "waveforms.h"

#define RBI "rbi-sciduino1k"
#define VER "0.5"


#if defined(ARDUINO_SAM_DUE) || defined (ARDUINO_GIGA)
#define ARM_ARDUINO
#define Serial SerialUSB
#endif

String serial_input = "";

WaveformArray waveforms;

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

u32 parse_channel_mask(String* channels) {
    if (*channels == "all") return -1;

    u32 mask = 0;
    while (*channels != "") {
        mask |= 1 << (u8((*channels)[0]) - u8('a'));
        channels->remove(0, 2);  // Remove channel ID and following comma
    }

    return mask;
}

// SCPI-like command handling
void process_single_command(String cmd) {
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

    if (try_pop_command(&cmd, ":inputs", ":in")) {
        if (try_pop_command(&cmd, ":enable", ":enab")) {
            adc->enable_inputs(parse_channel_mask(&cmd));
            return;
        }

        if (try_pop_command(&cmd, ":disable", ":disab")) {
            adc->disable_inputs(parse_channel_mask(&cmd));
            return;
        }

        if (try_pop_command(&cmd, ":set", ":set")) {
            adc->disable_inputs(-1);  // Disable all
            adc->enable_inputs(parse_channel_mask(&cmd));
            return;
        }

        if (try_pop_command(&cmd, "?", "?")) {
            if (transmission_format == TransmissionFormat::Binary) {
                Serial.write(ANALOG_INPUT_COUNT);
                Serial.write((char*) analog_inputs, sizeof(analog_inputs));
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

                    Serial.print(F(";range "));
                    Serial.print(analog_inputs[i].input_range_id);

                    Serial.print(F(";precision "));
                    Serial.print(analog_inputs[i].precision);
                    Serial.print(F(";pin "));
                    Serial.print(analog_inputs[i].pin);

                    Serial.print(F(";enabled "));
                    Serial.print(analog_inputs[i].enabled ? "true" : "false");

                    Serial.print(F( ";end" ));
                    Serial.print(i == ANALOG_INPUT_COUNT - 1 ? '\n' : ';');
                }
            }
            return;
        }

        if (try_pop_command(&cmd, ":ranges?", ":ran?")) {
            const GlobalInputRange* arr;
            size_t len;
            adc->getAvailableInputRanges(&arr, &len);
            if (transmission_format == TransmissionFormat::Binary) {
                Serial.write((u8) len);
                Serial.write((char*) arr, sizeof(GlobalInputRange) * len);
            }
            else {
                Serial.println("Err -- todo");
            }
            return;
        }
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
        // We push the response in a buffer to send it in one go, in part to
        // get fast and reliable transmission, but mostly to minimise the delay
        // between measurements.
        u8 buffer[2 + 3 * (ANALOG_INPUT_COUNT)];

        if (transmission_format == TransmissionFormat::Ascii) {
            Serial.println("Err -- todo");
        }
        else {
            buffer[0] = 'B';
            u8 active_input_count = 0;
            for (size_t i = 0; i < adc->input_count; i++) {
                if (adc->inputs[i].enabled) active_input_count++;
            }

            buffer[1] = active_input_count;

            size_t channels_read = 0;
            for (size_t i = 0; i < adc->input_count; i++) {
                if (adc->inputs[i].enabled) {
                    buffer[2 + channels_read * 3] = 'A' + i;
                    u16 value = adc->analogRead(i);
                    buffer[3 + channels_read * 3] = value >> 8;
                    buffer[4 + channels_read * 3] = value;
                    channels_read++;
                }
            }

            Serial.write((char*) buffer, 2 + 3 * channels_read);
        }
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

        adc->analogReadBurst(&waveforms, measurements, frequency);
        return;
    }

    if (try_pop_command(&cmd, ":stream", ":str")) {
        if (cmd.startsWith(":stop")) {
            timer_stop();
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

        adc->analogReadStream(&waveforms, 500, frequency);
        return;
    }

    Serial.print(F("ERR -- unsupported command: "));
    Serial.println(cmd);
}

String expand_abbreviations(const String& prev_cmd, const String& curr_cmd) {
    if (curr_cmd[0] == ':' || curr_cmd[0] == '*') return curr_cmd;
    size_t last_colon_index = prev_cmd.lastIndexOf(':');
    return prev_cmd.substring(0, last_colon_index) + curr_cmd;
}

void process_all_commands(String commands) {
    size_t semicolon_position;
    String prev_cmd = "";

    while ((semicolon_position = commands.indexOf(';')) != -1) {
        String curr_cmd = commands.substring(0, semicolon_position);
        process_single_command(expand_abbreviations(prev_cmd, curr_cmd));
        commands.remove(0, semicolon_position + 1);
        prev_cmd = curr_cmd;
    }

    process_single_command(expand_abbreviations(prev_cmd, commands));
}


void setup() {
    serial_input.reserve(256);
    Serial.begin(115200);
    adc->begin();
    while (!Serial);
}


void loop() {
    waveforms.process_scheduled_transmission();

    while (Serial.available()) {
        char input = (char) Serial.read();
        if (input == '\n') {
            process_all_commands(serial_input);
            serial_input = "";
        } else {
            serial_input += input;
        }
    }
}
