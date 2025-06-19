#include "waveforms.h"

#if defined(ARDUINO_SAM_DUE) || defined (ARDUINO_GIGA)
#define ARM_ARDUINO
#define Serial SerialUSB
#endif


FillStatus Waveform::push(u16 value) {
    this->data[this->current_index++] = value;

    if (this->current_index == this->meta.length) {
        this->current_index = 0;
        return FillStatus::CompletellyFull;
    }

    return this->current_index == this->meta.length / 2
        ? FillStatus::HalfFull
        : FillStatus::DontWorry
    ;
}


void Waveform::read_subset(const u16** ptr, size_t* len, BufferSubset subset) const {
    switch (subset) {
        case BufferSubset::Full: {
            *ptr = this->data;
            *len = this->meta.length;
            break;
        }

        case BufferSubset::FirstHalf: {
            *ptr = this->data;
            *len = this->meta.length / 2;
            break;
        }

        case BufferSubset::SecondHalf: {
            size_t halfway_point = this->meta.length / 2;
            *ptr = this->data + halfway_point;
            *len = this->meta.length - halfway_point;
            break;
        }
    }
}


size_t send_waveform_array_ascii(const Waveform* arr, size_t array_length, BufferSubset subset) {
    size_t len;
    const u16* buffer;
    char scientific_float_buffer[16];

    // XXX: sprintf weighs almost 10kB of progmem by itself, we might need a
    // lighter alternative for AVR boards
    Serial.print(":waveform:");
    for (size_t i = 0; i < array_length; i++) {
        arr[i].read_subset(&buffer, &len, subset);

        Serial.print(F("begin;length "));
        Serial.print(len);

        Serial.print(F(";time "));
        sprintf(scientific_float_buffer, "%.6e", arr[i].meta.time);
        Serial.print(scientific_float_buffer);

        Serial.print(F(";interval "));
        sprintf(scientific_float_buffer, "%.6e", arr[i].meta.interval);
        Serial.print(scientific_float_buffer);

        Serial.print(F(";pin "));
        Serial.print(arr[i].meta.pin);

        Serial.print(F(";data "));
        for (size_t i = 0; i < len - 1; i++) {
            Serial.print(buffer[i]);
            Serial.print(',');
        }
        Serial.print(buffer[len - 1]);
        Serial.print(F(";end"));
        Serial.print(i == array_length - 1 ? '\n' : ';');
    }

    return len;
}


size_t send_waveform_array_binary(const Waveform* arr, size_t array_length, BufferSubset subset) {
    size_t len;
    const u16* buffer;

    Serial.write(array_length);
    for (size_t i = 0; i < array_length; i++) {
        arr[i].read_subset(&buffer, &len, subset);
        WaveformHeader header = arr[i].meta;
        header.length = len;
        Serial.write((const char*) &header, sizeof(WaveformHeader));
        Serial.write((const char*) buffer, len * sizeof(u16));
    }

    return len;
}


template<size_t const ARRAY_LENGTH, size_t const BUFFER_SIZE>
void WaveformArray<ARRAY_LENGTH, BUFFER_SIZE>::process_scheduled_transmission() {
    if (!transmission.is_scheduled) return;
    this->transmission.is_scheduled = false;

    Serial.print((char) this->transmission.format);
    auto fn = this->transmission.format == TransmissionFormat::Ascii
        ? send_waveform_array_ascii
        : send_waveform_array_binary
    ;

    size_t values_sent = fn(this->arr, this->active_count, this->transmission.subset);

    for (size_t i = 0; i < this->active_count; i++)
        // this->arr[i].meta.time += this->arr[i].meta.length * this->arr[i].meta.interval;
        this->arr[i].meta.time += values_sent * this->arr[i].meta.interval;
}

template<size_t const ARRAY_LENGTH, size_t const BUFFER_SIZE>
bool WaveformArray<ARRAY_LENGTH, BUFFER_SIZE>::add_waveform(WaveformHeader header) {
    size_t alloc_size = header.length * sizeof(u16);

    if (alloc_size > static_arena.available()) return false;

    this->arr[this->active_count++] = Waveform {
        .meta = header,
        .current_index = 0,
        .data = (u16*) this->static_arena.alloc(alloc_size),
    };

    return true;
}
