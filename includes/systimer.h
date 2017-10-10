#ifndef __SYSTIMER_H__
#define __SYSTIMER_H__

#include <stdint.h>

#define SYSTIMER_BASE 0x3F003000

typedef struct {
    volatile uint32_t control_status;
    volatile uint32_t counter_lo;
    volatile uint32_t counter_hi;
    volatile uint32_t compare0;
    volatile uint32_t compare1;
    volatile uint32_t compare2;
    volatile uint32_t compare3;
} sys_timer_t;

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

#endif