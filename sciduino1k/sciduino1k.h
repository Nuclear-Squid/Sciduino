// #include <avr/pgmspace.h>

#pragma once

#include "stdint_aliases.h"
#include "adc.h"

// #define WAVEFORM_BUFFER_BYTE_SIZE 50000
// #define WAVEFORM_BUFFER_BYTE_SIZE 5000

// // const AnalogInput analog_inputs[] PROGMEM = {
// AnalogInput analog_inputs[] = {
//     { "GBF (base)",    "V",  2 * 2.5 / 65536, 0,   4, 0 },
//     // { "GBF (base)",    "V",  3.3 / 1024, 0,   2, A0 },
//     // { "GBF (inverse)", "V", -3.3 / 1024, 3.3, 2, A2 },
// };

AnalogInput analog_inputs[] = {
    { "GBF (base)", "V", (u8) LTC1859::InputRange::ZeroTo2Vref, 4, 0 },
};

#define ANALOG_INPUT_COUNT sizeof(analog_inputs) / sizeof(AnalogInput)

// SciduinoADC* adc = new AnalogPins(analog_inputs, ANALOG_INPUT_COUNT);
// SciduinoADC* adc = new MAX1300(analog_inputs, ANALOG_INPUT_COUNT);
SciduinoADC* adc = new LTC1859(analog_inputs, ANALOG_INPUT_COUNT);
