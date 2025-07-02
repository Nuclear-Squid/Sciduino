#pragma once

#include "stdint_aliases.h"
#include "adc.h"

AnalogInput analog_inputs[] = {
    { "GBF (base)",   "V", (u8) LTC1859::InputRange::PlusMinus2Vref, 4, 0b000, true },
    { "fake input 1", "V", (u8) LTC1859::InputRange::PlusMinus2Vref, 4, 0b001, false },
    { "fake input 2", "V", (u8) LTC1859::InputRange::PlusMinus2Vref, 4, 0b010, false },
};

#define ANALOG_INPUT_COUNT sizeof(analog_inputs) / sizeof(AnalogInput)

// SciduinoADC* adc = new AnalogPins(analog_inputs, ANALOG_INPUT_COUNT);
// SciduinoADC* adc = new MAX1300(analog_inputs, ANALOG_INPUT_COUNT);
SciduinoADC* adc = new LTC1859(analog_inputs, ANALOG_INPUT_COUNT);
