#pragma once

#include <SPI.h>

#include "stdint_aliases.h"

// #define SPI_DEBUG

// Arduino Giga
#if 0
#define CS_PIN 10
#define Serial SerialUSB
#define SPI SPI1
#endif

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


class MAX1300 {
    const u8 resolution = 16;
    u8 cs_pin;
    bool debug;
    InputRange input_range;
    f32 vref;

public:
    void begin(int cs_pin, InputRange input_range, f32 vref=4.096, bool debug=false) {
        this->cs_pin = cs_pin;
        this->input_range = input_range;
        this->vref = vref;
        this->debug = debug;

        SPI.begin();
        pinMode(cs_pin, OUTPUT);
        digitalWrite(cs_pin, HIGH);

        SPI.begin();
        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

        this->setState(ADCState::Reset);
        this->setState(ADCState::ExternalClock);
        for (size_t i = 0; i < 8; i++)
            this->configureChannel(i, ChannelMode::SingleEnded, input_range);
    }

    void setState(ADCState state) {
        u8 command = (1 << 7) | (u8(state) << 4) | (1 << 3);
        if (this->debug) Serial.print(command, BIN);
        digitalWrite(this->cs_pin, LOW);
        SPI.transfer(command);
        SPI.transfer(0);
        digitalWrite(this->cs_pin, HIGH);
    }

    void configureChannel(u8 channel, ChannelMode mode, InputRange range) {
        u8 command = (1 << 7) | ((channel & 0b111) << 4) | (u8(mode) << 3) | u8(range);
        if (this->debug) Serial.print(command, BIN);
        digitalWrite(this->cs_pin, LOW);
        SPI.transfer16(command << 8);
        digitalWrite(this->cs_pin, HIGH);
    }

    u16 analogRead(u8 channel) {
        u8 command = (1 << 7) | ((channel & 0b111) << 4);
        if (this->debug) Serial.print(command, BIN);
        digitalWrite(this->cs_pin, LOW);
        SPI.transfer16(command << 8);
        u16 rv = SPI.transfer16(0);
        digitalWrite(this->cs_pin, HIGH);
        return rv;
    }

    f32 analogToFloat(u16 analog_value) {
        f32 full_scale_range, offset;
        using IR = InputRange;
        switch (this->input_range) {
            case IR::Centered3HalfVref:
            case IR::Negative3HalfVref:
            case IR::Positive3HalfVref:
                full_scale_range = this->vref;
                break;

            case IR::Centered3Vref:
            case IR::Negative3Vref:
            case IR::Positive3Vref:
                full_scale_range = this->vref * 2;
                break;

            case IR::Centered6Vref: full_scale_range = this->vref * 4; break;
        }

        switch (this->input_range) {
            case IR::Positive3HalfVref:
            case IR::Positive3Vref:
                offset = 0;
                break;

            case IR::Centered3HalfVref:
            case IR::Centered3Vref:
            case IR::Centered6Vref:
                offset = full_scale_range / 2;
                break;

            case IR::Negative3HalfVref:
            case IR::Negative3Vref:
                offset = full_scale_range;
                break;
        }

        return f32(analog_value) * full_scale_range / pow(2, this->resolution) - offset;
    }
};


// MAX1300 adc;
//
// void setup() {
//     Serial.begin(115200);
//     while (!Serial);
//     pinMode(A0, INPUT);
//     adc.begin(10, InputRange::Positive3HalfVref);
// }
//
//
// void loop() {
//
//     // Read channel every 100ms, compare to A0 pin
//     #if 1
//     Serial.print("AnalogPin: ");
//     Serial.print(analogRead(A0) * 3.3 / 1023);
//     Serial.print("; MAX1300: ");
//     Serial.print(adc.analogToFloat(adc.analogRead(0)));
//     Serial.println();
//     delay(100);
//
//     // Benchmark
//     #else
//
//     const size_t ITER_COUNT = 10000;
//     unsigned long start_time = micros();
//     for (size_t i = 0; i < ITER_COUNT; i++) {
//         u16 spi_data = adc_read_channel(0);
//         if (spi_data < 28000) {
//             Serial.println("Transmission Error");
//         }
//     }
//     unsigned long end_time = micros();
//     char buffer[64];
//     // sprintf(buffer, "%.6e", double(end_time - start_time) / 10000);
//     Serial.println(double(end_time - start_time) / ITER_COUNT);
//
//     #endif
// }
