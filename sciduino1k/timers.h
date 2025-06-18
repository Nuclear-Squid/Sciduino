#pragma once

#if defined(__SAM3X8E__)

#include <DueTimer.h>

void timer_attach_interrupt(void(*isr)(), double frequency) {
    Timer1.attachInterrupt(isr).setFrequency(frequency).start();
}

void timer_stop() {
    Timer1.stop();
}

#else

#include "3rd_part/TimerInterrupt.h"

void timer_attach_interrupt(void(*isr)(), double frequency) {
    ITimer1.attachInterrupt(frequency, isr);
}

void timer_stop() {
    ITimer1.stopTimer();
}


#endif
