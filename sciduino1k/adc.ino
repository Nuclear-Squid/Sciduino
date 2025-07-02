#include "adc.h"
#include "waveforms.h"

void SciduinoADC::analogReadBurst(WaveformArray* waveforms, size_t measurements, float frequency) {
    waveforms->clear();
    for (size_t i = 0; i < this->input_count; i++) {
        if (!this->inputs[i].enabled) continue;
        WaveformHeader header = {
            .length = measurements,
            .time = 0,
            .interval = 1 / frequency,
            .pin = analog_inputs[i].pin,
        };

        if (!waveforms->add_waveform(header)) {
            Serial.println("Err -- could not allocate enough memory for waveforms");
            waveforms->clear();
            return;
        }
    }

    auto timer_handler_burst = []() {
        extern SciduinoADC* adc;
        extern WaveformArray waveforms;

        FillStatus status;
        for (size_t i = 0; i < waveforms.active_count; i++) {
            status = waveforms.arr[i].push(adc->analogRead(waveforms.arr[i].meta.pin));
        }

        if (status == FillStatus::CompletellyFull) {
            timer_stop();
            waveforms.schedule_transmission(transmission_format, BufferSubset::Full);
        }
    };

    timer_attach_interrupt(timer_handler_burst, frequency);
}


void SciduinoADC::analogReadStream(WaveformArray* waveforms, size_t measurements, float frequency) {
    waveforms->clear();
    for (size_t i = 0; i < this->input_count; i++) {
        if (!this->inputs[i].enabled) continue;
        WaveformHeader header = {
            .length = measurements,
            .time = 0,
            .interval = 1 / frequency,
            .pin = analog_inputs[i].pin,
        };

        if (!waveforms->add_waveform(header)) {
            Serial.println("Err -- could not allocate enough memory for waveforms");
            waveforms->clear();
            return;
        }
    }

    auto timer_handler_stream = []() {
        extern SciduinoADC* adc;
        extern WaveformArray waveforms;

        FillStatus status;
        for (size_t i = 0; i < waveforms.active_count; i++) {
            status = waveforms.arr[i].push(adc->analogRead(waveforms.arr[i].meta.pin));
        }

        switch (status) {
            case FillStatus::DontWorry: break;

            case FillStatus::HalfFull:
                waveforms.schedule_transmission(transmission_format, BufferSubset::FirstHalf);
                break;

            case FillStatus::CompletellyFull:
                waveforms.schedule_transmission(transmission_format, BufferSubset::SecondHalf);
                break;
        }
    };

    timer_attach_interrupt(timer_handler_stream, frequency);
}


void MAX1300::begin() {
    SPI.begin();
    pinMode(this->cs_pin, OUTPUT);
    digitalWrite(this->cs_pin, HIGH);

    SPI.begin();
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    this->setState(MAX1300::State::Reset);
    this->setState(MAX1300::State::ExternalClock);
    for (size_t i = 0; i < 8; i++)
        this->configureChannel(i, ChannelMode::SingleEnded, input_range);
}

void MAX1300::setState(MAX1300::State state) {
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
    auto a = MAX1300::InputRange::Centered3HalfVref;
    Serial.println(byte(a));
    using IR = MAX1300::InputRange;
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


void LTC1859::begin() {
    SPI.begin();
    SPI.beginTransaction(SPISettings(25000000, MSBFIRST, SPI_MODE0));

    pinMode(this->cs_pin, OUTPUT);
    digitalWrite(this->cs_pin, HIGH);

    pinMode(this->conversion_start_pin, OUTPUT);
    digitalWrite(this->conversion_start_pin, LOW);

    pinMode(this->busy_pin, INPUT);
}


u16 LTC1859::analogReadFast(u8 channel) {
    LTC1859::SpiCommand command = {
        .power = 0,
        .input_range = (u8) this->inputs[channel].input_range_id,
        .channel = channel,
        .single_ended = true,
    };

    digitalWrite(this->cs_pin, LOW);
    u16 rv = SPI.transfer16(command.to_byte() << 8);
    digitalWrite(this->cs_pin, HIGH);

    return rv;
}
