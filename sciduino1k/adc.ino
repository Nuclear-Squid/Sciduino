#include "adc.h"


void MAX1300::begin(int cs_pin, InputRange input_range, f32 vref, bool debug) {
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

void MAX1300::setState(ADCState state) {
    u8 command = (1 << 7) | (u8(state) << 4) | (1 << 3);
    if (this->debug) Serial.print(command, BIN);
    digitalWrite(this->cs_pin, LOW);
    SPI.transfer(command);
    SPI.transfer(0);
    digitalWrite(this->cs_pin, HIGH);
}

void MAX1300::configureChannel(u8 channel, ChannelMode mode, InputRange range) {
    u8 command = (1 << 7) | ((channel & 0b111) << 4) | (u8(mode) << 3) | u8(range);
    if (this->debug) Serial.print(command, BIN);
    digitalWrite(this->cs_pin, LOW);
    SPI.transfer16(command << 8);
    digitalWrite(this->cs_pin, HIGH);
}

u16 MAX1300::analogRead(u8 channel) {
    u8 command = (1 << 7) | ((channel & 0b111) << 4);
    if (this->debug) Serial.print(command, BIN);
    digitalWrite(this->cs_pin, LOW);
    SPI.transfer16(command << 8);
    u16 rv = SPI.transfer16(0);
    digitalWrite(this->cs_pin, HIGH);
    return rv;
}

f32 MAX1300::analogToFloat(u16 analog_value) {
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
