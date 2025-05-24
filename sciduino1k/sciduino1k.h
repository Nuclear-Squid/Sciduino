#include <avr/pgmspace.h>

#include "stdint_aliases.h"

#define WAVEFORM_BUFFER_BYTE_SIZE 50000

// We pack the structs to make sure there are no difference in padding / alignment
// between 16 bits AVR, 32 bits ARM and 64 bits desktops when transmitting them
// as binary data.
typedef struct __attribute__ ((packed)) {
    char name[16];
    char unit[8];
    f32  gain;
    f32  offset;
    u8   precision;
    u8   pin;
} AnalogInput;

const AnalogInput analog_inputs[] PROGMEM = {
    { "GBF (base)",    "V",  3.3 / 1024, 0,   2, A0 },
    { "GBF (inverse)", "V", -3.3 / 1024, 3.3, 2, A2 },
};

#define ANALOG_INPUT_COUNT sizeof(analog_inputs) / sizeof(AnalogInput)
