#pragma once

#include "stdint_aliases.h"

template <class T, size_t const SIZE>
class FlipFlopBuffer {
private:
    u8 current_write_buffer;
    T  buffer1[SIZE];
    T  buffer2[SIZE];

public:
    T* get_write_buffer() {
        return this->current_write_buffer == 0
            ? this->buffer1
            : this->buffer2
        ;
    }

    const T* get_read_buffer() const {
        return this->current_write_buffer == 0
            ? this->buffer2
            : this->buffer1
        ;
    }

    inline void flip_buffers() {
        this->current_write_buffer ^= 1;
    }

    inline size_t length() const {
        return SIZE;
    }
};
