#pragma once

#if defined(ARDUINO_SAM_DUE)

    #include <DueTimer.h>

    void timer_attach_interrupt(void(*isr)(), double frequency) {
        Timer1.attachInterrupt(isr).setFrequency(frequency).start();
    }

    void timer_stop() {
        Timer1.stop();
    }

#else
    #if defined(ARDUINO_GIGA)
        // #include "3rd_part/Portenta_H7_TimerInterrupt/src/Portenta_H7_TimerInterrupt.h"
        // #include <Portenta_H7_TimerInterrupt.h>
        #include <TimerInterrupt_Generic.h>
        static Portenta_H7_TimerInterrupt ITimer1(TIM8);
    #else
        #include "3rd_part/TimerInterrupt.h"
    #endif

    void timer_attach_interrupt(void(*isr)(), double frequency) {
        ITimer1.attachInterrupt(frequency, isr);
    }

    void timer_stop() {
        ITimer1.stopTimer();
    }
#endif
