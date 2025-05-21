#include <avr/pgmspace.h>

#include "stdint_aliases.h"

typedef struct {
    char name[16];
    char unit[8];
    f32  gain;
    f32  offset;
    u8   precision;
    u8   pin;

    void print_name() const { Serial.print((__FlashStringHelper*) this->name); }
    void println_name() const { Serial.println((__FlashStringHelper*) this->name); }

    void print_unit() const { Serial.print((__FlashStringHelper*) this->unit); }
    void println_unit() const { Serial.println((__FlashStringHelper*) this->unit); }

    f32 get_gain()      const { return pgm_read_float(&this->gain); }
    f32 get_offset()    const { return pgm_read_float(&this->offset); }
    u8  get_precision() const { return pgm_read_byte(&this->precision); }
    u8  get_pin()       const { return pgm_read_byte(&this->pin); }
} AnalogInput;

const AnalogInput analog_inputs[] PROGMEM = {
    { "GBF (base)",    "V", 3.3  / 1024.0, 0,   2, A0 },
    { "GBF (inverse)", "V", -3.3 / 1024.0, 3.3, 2, A2 },
};

typedef struct {
    f32 initial_time;
    f32 time_interval;
    u32 values_count;
    u8  pin;
} WaveformHeader;
