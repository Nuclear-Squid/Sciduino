#pragma once

#include "stdint_aliases.h"

enum class TransmissionFormat: char {
    Ascii  = 'A',
    Binary = 'B',
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

template<size_t const BUFFER_SIZE>
struct StaticArenaAllocator {
    size_t currently_allocated;
    u8 buffer[BUFFER_SIZE];

    inline size_t available() { return BUFFER_SIZE - currently_allocated; }
    inline void clear() { currently_allocated = 0; }

    u8* alloc(size_t size) {
        if (size > BUFFER_SIZE - currently_allocated) return nullptr;
        u8* rv = ((u8*) buffer) + currently_allocated;
        currently_allocated += size;
        return rv;
    }
};

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
    struct StaticArenaAllocator<BUFFER_SIZE> static_arena;

    struct {
        TransmissionFormat format;
        BufferSubset subset;
        bool is_scheduled;
    } transmission;

    bool add_waveform(WaveformHeader);
    void process_scheduled_transmission();

    inline void schedule_transmission(TransmissionFormat format, BufferSubset subset) {
        this->transmission = { format, subset, true };
    }

    void clear() {
        this->static_arena.clear();
        this->active_count = 0;
    }
};
