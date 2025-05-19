#include "stdint_aliases.h"

typedef struct {
    char name[16];
    char unit[8];
    u8   pin;
    u8   precision;
    f32  gain;
    f32  offset;
} AnalogInput;

const AnalogInput analog_inputs[] = {
    { "GBF", "V", A0, 2, 3.3 / 1024, 0 },
};

typedef struct {
    f32 initial_time;
    f32 time_interval;
    u32 values_count;
    u8  pin_index;
} WaveformHeader;
