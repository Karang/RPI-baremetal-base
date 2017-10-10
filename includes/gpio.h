#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdint.h>

typedef enum {
    LOW  = 0x0,
    HIGH = 0x1
} GpioState;

typedef enum {
    PULL_DISABLE= 0x0,
    PULL_DOWN   = 0x1,
    PULL_UP     = 0x2
} GpioResistor;

typedef enum {
    INPUT   = 0x0,
    OUTPUT  = 0x1,
    ALT_0   = 0x4,
    ALT_1   = 0x5,
    ALT_2   = 0x6,
    ALT_3   = 0x7,
    ALT_4   = 0x3,
    ALT_5   = 0x2
} GpioFunction;

void gpio_set_mode(int gpio, GpioFunction mode);
void gpio_set_resistor(int gpio, GpioResistor resistor);
void gpio_write(int gpio, GpioState state);

#endif