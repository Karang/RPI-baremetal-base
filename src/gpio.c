#include "gpio.h"

#define GPIO_BASE (volatile uint32_t *)(0x3F200000) // 0x20200000 pour rpi1

#define GPPUD       *(GPIO_BASE + 0x94)
#define GPPUDCLK0   *(GPIO_BASE + 0x98)

/* Helper Macros */

#define GPIO_CONF_REG(addr, gpio) \
    *(GPIO_BASE + (((addr) / sizeof(uint32_t)) + ((gpio) / 32)))

#define GPIO_SET(gpio) \
    GPIO_CONF_REG(0x1c, gpio) = 1 << ((gpio) % 32)
    
#define GPIO_CLR(gpio) \
    GPIO_CONF_REG(0x28, gpio) = 1 << ((gpio) % 32)

#define GPIO_VALUE(gpio) \
    ((GPIO_CONF_REG(0x34, gpio) >> ((gpio) % 32)) & 0x1) 



void gpio_set_mode(int gpio, GpioFunction mode) {
    // Clear what ever function bits currently exist - this puts the pin
    // into input mode.
    *(GPIO_BASE + (gpio/10)) &= ~(0x7 << ((gpio % 10) * 3));

    // Set the three pins for the pin to the desired value
    *(GPIO_BASE + (gpio/10)) |= (mode << ((gpio % 10) * 3));
}

void gpio_set_resistor(int gpio, GpioResistor resistor) {
    volatile int i;
    
    GPPUD = resistor;
    
    /* Wait for control signal to be set up */
    for (i=0; i<150 ; i++) {}
    
    /* Clock the control signal for desired resistor */
    GPPUDCLK0 = (0x1 << gpio);
    
    /* Hold to set */
    for (i=0; i<150 ; i++) {}
    
    GPPUD = 0;
    GPPUDCLK0 = 0;
}

void gpio_write(int gpio, GpioState state) {
    if (state == HIGH) GPIO_SET(gpio);
    else GPIO_CLR(gpio);
}