#pragma once

#include <cstddef>
#include "stdint_aliases.h"

enum class TransmissionFormat: u8 {
    Ascii,
    Binary,
};

enum class FillStatus: u8 {
    DontWorry,
    HalfFull,
    CompletellyFull,
};

enum class BufferSubset: u8 {
    Full,
    FirstHalf,
    SecondHalf,
};

typedef struct __attribute__ ((packed)) {
    u32 length;
    f32 time;
    f32 interval;
    u8  pin;
} WaveformHeader;

//          ╭─────────────────────────────────────────────────────────╮
//          │                        Waveform                         │
//          ╰─────────────────────────────────────────────────────────╯

typedef struct Waveform {
    WaveformHeader meta;
    size_t current_index;
    u16* data;

    FillStatus push(u16 value);
    void read_subset(const u16** ptr, size_t* len, BufferSubset subset) const;
} Waveform;


template<size_t const ARRAY_LENGTH, size_t const BUFFER_SIZE>
struct WaveformArray {
    size_t active_count;
    Waveform arr[ARRAY_LENGTH];

    struct {
        TransmissionFormat format;
        BufferSubset subset;
        bool is_scheduled;
    } transmission;

    struct {
        size_t currently_allocated;
        u8 buffer[BUFFER_SIZE];

        inline size_t available() { return BUFFER_SIZE - currently_allocated; }
        u8* alloc(size_t size) {
            if (size > BUFFER_SIZE - currently_allocated) return nullptr;
            u8* rv = ((u8*) buffer) + currently_allocated;
            currently_allocated += size;
            return rv;
        }
    } static_arena;

    bool add_waveform(WaveformHeader);
    void process_scheduled_transmission();

    inline void schedule_transmission(TransmissionFormat format, BufferSubset subset) {
        this->transmission = { format, subset, true };
    }

    void clear() {
        this->active_count = 0;
        this->static_arena.currently_allocated = 0;
    }
};
