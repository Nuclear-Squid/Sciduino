#pragma once

#include <SPI.h>

#include "stdint_aliases.h"
#include "waveforms.h"

// #define SPI_DEBUG

// Arduino Giga
// #if 0
// #define CS_PIN 10
#define Serial SerialUSB
// #define SPI SPI1
// #endif

// clang-format off
enum class ADCState: u8 {
    ExternalClock       = 0b000,
    ExternalAcquisition = 0b001,
    InternalClock       = 0b010,
    Reset               = 0b100,
    PartialPowerDown    = 0b110,
    FullPowerDown       = 0b111,
};

enum class ChannelMode: u8 {
    SingleEnded  = 0,
    Differential = 1,
};

enum class InputRange: u8 {
    Centered3HalfVref = 0b001,
    Negative3HalfVref = 0b010,
    Positive3HalfVref = 0b011,
    Centered3Vref     = 0b100,
    Negative3Vref     = 0b101,
    Positive3Vref     = 0b110,
    Centered6Vref     = 0b111,
};
// clang-format on


class SciduinoADC {
public:
    virtual u16 analogRead(u8 channel) = 0;
    virtual f32 analogToFloat(u16 analog_value) = 0;

    // template<size_t const ARRAY_LENGTH, size_t const BUFFER_SIZE>
    // void analogReadBurst(WaveformArray<ARRAY_LENGTH, BUFFER_SIZE>* waveforms, size_t measurements, float frequency);

// protected:
//     static void timerHandlerBurst();
};

class AnalogPins: public SciduinoADC {
    // void begin(int cs_pin, InputRange input_range, f32 vref=4.096, bool debug=false);
    // void setState(ADCState state);
    // void configureChannel(u8 channel, ChannelMode mode, InputRange range);

    u16 analogRead(u8 channel) {
        // Explicitaly use analogRead from global namespace, otherwise
        // it causes an infinite recursion.
        return ::analogRead(channel);
    }

    f32 analogToFloat(u16 analog_value) { return analog_value * 3.3 / 1024; }
};

class MAX1300: public SciduinoADC {
    const u8 resolution = 16;
    u8 cs_pin;
    bool debug;
    InputRange input_range;
    f32 vref;

public:
    void begin(int cs_pin, InputRange input_range, f32 vref=4.096, bool debug=false);
    void setState(ADCState state);
    void configureChannel(u8 channel, ChannelMode mode, InputRange range);
    u16 analogRead(u8 channel);
    f32 analogToFloat(u16 analog_value);
};

class LTC1859: public SciduinoADC {
public:
    const u8 resolution{16};
    const u8 cs_pin{10};
    const u8 conversion_start_pin{9};
    const u8 busy_pin{3};

    void begin() {
        SPI.begin();
        pinMode(this->cs_pin, OUTPUT);
        digitalWrite(this->cs_pin, HIGH);

        pinMode(this->conversion_start_pin, OUTPUT);
        digitalWrite(this->conversion_start_pin, LOW);

        pinMode(this->busy_pin, INPUT);
    }

    u16 analogRead(u8 channel) {
        static u8 command =
            (1 << 7)  // single ended
            | ((channel & 0b111) << 4)  // Channel select
            | (0b00 << 2)  // Input range: 0-5V
            | 0b00  // Keep awake after operation.
        ;

        Serial.print("cmd: ");
        Serial.print(command, BIN);
        Serial.print("; value: ");

        digitalWrite(this->conversion_start_pin, HIGH);
        while (digitalRead(this->busy_pin) == LOW) {}
        digitalWrite(this->conversion_start_pin, LOW);
        digitalWrite(this->cs_pin, LOW);

        SPI.transfer16(command << 8);

        digitalWrite(this->cs_pin, HIGH);


        digitalWrite(this->conversion_start_pin, HIGH);
        while (digitalRead(this->busy_pin) == LOW) {}
        digitalWrite(this->conversion_start_pin, LOW);
        digitalWrite(this->cs_pin, LOW);

        u16 rv = SPI.transfer16(command << 8);

        digitalWrite(this->cs_pin, HIGH);

        return rv;
    }

    f32 analogToFloat(u16 analog_value) {
        return analog_value * 5 / 65536;
    }
};

// vim:ft=arduino
