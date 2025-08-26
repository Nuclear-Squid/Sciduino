#pragma once

#include <SPI.h>

#include "stdint_aliases.h"
#include "waveforms.h"

// #define SPI_DEBUG

// Arduino Giga
// #if 0
// #define CS_PIN 10
// #define Serial SerialUSB
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
    u8   precision;
    u8   pin;
    bool enabled;
} AnalogInput;
// clang-format on

typedef struct {
    f32 gain;
    f32 offset;
} GlobalInputRange;

class SciduinoADC {
public:
    AnalogInput* inputs;
    const size_t input_count;

    SciduinoADC(AnalogInput* inputs, size_t input_count): inputs(inputs), input_count(input_count) {}

    virtual void begin() = 0;
    virtual u16 analogRead(u8 channel) = 0;
    virtual void analogReadBurst(WaveformArray* waveforms, size_t measurements, float frequency);
    virtual void analogReadStream(WaveformArray* waveforms, size_t measurements, float frequency);
    virtual f32 analogToFloat(u16 analog_value) = 0;

    // virtual const GlobalInputRange* getAvailableInputRanges() = 0;
    virtual void getAvailableInputRanges(const GlobalInputRange** arr, size_t* len) = 0;


    GlobalInputRange getInputRange(u8 channel) {
        const GlobalInputRange* arr;
        size_t len;
        // return this->getAvailableInputRanges()[this->inputs[channel].input_range_id];
        this->getAvailableInputRanges(&arr, &len);
        return arr[this->inputs[channel].input_range_id];
    }

    void enable_inputs(u32 input_mask) {
        for (size_t i = 0; i < this->input_count; i++)
            if (input_mask & (1 << i)) this->inputs[i].enabled = true;
    }

    void disable_inputs(u32 input_mask) {
        for (size_t i = 0; i < this->input_count; i++)
            if (input_mask & (1 << i)) this->inputs[i].enabled = false;
    }
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
public:
    typedef struct {
        u8 power: 2;
        u8 input_range: 2;
        u8 channel: 3;
        bool single_ended: 1;
        inline u8 to_byte() { return * (u8*) this; }
    } SpiCommand;


    enum class InputRange: u8 {
        PlusMinus2Vref = 0b00,
        PlusMinus4Vref = 0b01,
        ZeroTo2Vref    = 0b10,
        ZeroTo4Vref    = 0b11,
    };

    const float vref{2.5};
    const u8 resolution{16};
    const u8 cs_pin{10};
    const u8 conversion_start_pin{9};
    const u8 busy_pin{3};

    using SciduinoADC::SciduinoADC;

    void begin();
    u16 analogReadFast(u8 channel);

    inline u16 analogRead(u8 channel) {
        // Run the command twice, since the first will return the previous measurement.
        this->analogReadFast(channel);
        return this->analogReadFast(channel);
    }

    f32 analogToFloat(u16 analog_value) { return analog_value * 5 / 65536; }

    // const GlobalInputRange* getAvailableInputRanges() { return this->available_input_ranges; }
    void getAvailableInputRanges(const GlobalInputRange** arr, size_t* len) {
        using IR = LTC1859::InputRange;
        const f32 step = this->vref / pow(2, this->resolution);
        static const GlobalInputRange available_input_ranges[] = {
            [(u8) IR::PlusMinus2Vref] = { .gain = 4 * step, .offset = -2 * this->vref },
            [(u8) IR::PlusMinus4Vref] = { .gain = 8 * step, .offset = -4 * this->vref },
            [(u8) IR::ZeroTo2Vref]    = { .gain = 2 * step, .offset = 0 },
            [(u8) IR::ZeroTo4Vref]    = { .gain = 4 * step, .offset = 0 },
        };

        *arr = available_input_ranges;
        *len = sizeof(available_input_ranges) / sizeof(GlobalInputRange);
    }
};

// vim:ft=arduino
