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
// We pack the structs to make sure there are no difference in padding / alignment
// between 16 bits AVR, 32 bits ARM and 64 bits desktops when transmitting them
// as binary data.
typedef struct __attribute__ ((packed)) AnalogInput {
    char name[16];
    char unit[8];
    u8   input_range_id;
    // f32  gain;
    // f32  offset;
    u8   precision;
    u8   pin;
} AnalogInput;
// clang-format on

typedef struct {
    f32 gain;
    f32 offset;
    u8 code;
} GlobalInputRange;

class SciduinoADC {
public:
    AnalogInput* inputs;
    const size_t input_count;

    SciduinoADC(AnalogInput* inputs, size_t input_count): inputs(inputs), input_count(input_count) {}

    virtual void begin() = 0;
    virtual u16 analogRead(u8 channel) = 0;
    // virtual u16 analogReadFast(u8 channel) { this->analogRead(channel); }
    virtual f32 analogToFloat(u16 analog_value) = 0;

    virtual const GlobalInputRange* getAvailableInputRanges() = 0;
    GlobalInputRange getInputRange(u8 channel) {
        return this->getAvailableInputRanges()[this->inputs[channel].input_range_id];
    }

    // template<size_t const ARRAY_LENGTH, size_t const BUFFER_SIZE>
    // void analogReadBurst(WaveformArray<ARRAY_LENGTH, BUFFER_SIZE>* waveforms, size_t measurements, float frequency);

// protected:
//     static void timerHandlerBurst();
};

class AnalogPins: public SciduinoADC {
    // void begin(int cs_pin, InputRange input_range, f32 vref=4.096, bool debug=false);
    // void setState(ADCState state);
    // void configureChannel(u8 channel, ChannelMode mode, InputRange range);

    using SciduinoADC::SciduinoADC;

    void begin() {
        for (auto i = 0; i < this->input_count; i++) pinMode(this->inputs[i].pin, INPUT);
    }

    u16 analogRead(u8 channel) {
        // Explicitaly use analogRead from global namespace, otherwise
        // it causes an infinite recursion.
        return ::analogRead(channel);
    }

    f32 analogToFloat(u16 analog_value) {
        return analog_value * 3.3 / 1024;
    }
};

class MAX1300: public SciduinoADC {
public:
    // clang-format off
    enum class State: u8 {
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
    const u8 resolution = 16;
    u8 cs_pin;
    bool debug;
    InputRange input_range;
    f32 vref;

    using SciduinoADC::SciduinoADC;
    void begin();
    void setState(State state);
    void configureChannel(u8 channel, ChannelMode mode, InputRange range);
    u16 analogRead(u8 channel);
    f32 analogToFloat(u16 analog_value);
};

class LTC1859: public SciduinoADC {
protected:
    typedef struct {
        u8 power: 2;
        u8 input_range: 2;
        u8 channel: 3;
        bool single_ended: 1;
        u8 to_byte() { return * (u8*) this; }
    } SpiCommand;

public:

    enum class InputRange: u8 {
        ZeroTo2Vref,
        ZeroTo4Vref,
        PlusMinus2Vref,
        PlusMinus4Vref,
    };

    const float vref{2.5};
    const GlobalInputRange available_input_ranges[4] = {
        [(u8) LTC1859::InputRange::ZeroTo2Vref]
            = { .gain = 2 * this->vref / pow(2, 16), .offset = 0, .code = 0b10 },

        [(u8) LTC1859::InputRange::ZeroTo4Vref]
            = { .gain = 4 * this->vref / pow(2, 16), .offset = 0, .code = 0b11 },

        [(u8) LTC1859::InputRange::PlusMinus2Vref]
            = { .gain = 4 * this->vref / pow(2, 16), .offset = -2 * this->vref, .code = 0b00 },

        [(u8) LTC1859::InputRange::PlusMinus4Vref]
            = { .gain = 8 * this->vref / pow(2, 16), .offset = -4 * this->vref, .code = 0b01 },
    };

    const u8 resolution{16};
    const u8 cs_pin{10};
    const u8 conversion_start_pin{9};
    const u8 busy_pin{3};

    using SciduinoADC::SciduinoADC;

    void begin();
    u16 analogReadFast(u8 channel);

    u16 analogRead(u8 channel) {
        // Run the command twice, since the first will return the previous measurement.
        this->analogReadFast(channel);
        return this->analogReadFast(channel);
    }

    f32 analogToFloat(u16 analog_value) { return analog_value * 5 / 65536; }
    const GlobalInputRange* getAvailableInputRanges() { return this->available_input_ranges; }
};

// vim:ft=arduino
