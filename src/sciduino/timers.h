#pragma once

#include "3rd_part/TimerInterrupt_Generic.h"

#if defined(ARDUINO_AVR_NANO_EVERY)
    void timer_attach_interrupt(void(*isr)(), double frequency) {
        ITimer1.attachInterrupt(frequency, isr);
    }

    void timer_stop() {
        ITimer1.stopTimer();
    }

    void timer_init() {
        ITimer1.init();
    }

#elif defined(ARDUINO_SAM_DUE)
    void timer_attach_interrupt(void(*isr)(), double frequency) {
        Timer1.attachInterrupt(isr).setFrequency(frequency).start();
    }

    void timer_stop() {
        Timer1.stop();
    }

    void timer_init() { }

#else
// TODO: Provide a software fallback in case we fail to provide a haardware
// timer implementation.
#error "Timers are not supported for this board."

#endif

