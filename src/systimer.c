#include "systimer.h"

static sys_timer_t* systimer = (sys_timer_t*) SYSTIMER_BASE;

void delay_us(uint32_t us) {
    volatile uint32_t ts = systimer->counter_lo;
    while ((systimer->counter_lo - ts) < us) { }
}

void delay_ms(uint32_t ms) {
    delay_us(ms*1000);
}