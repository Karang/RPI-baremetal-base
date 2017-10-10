#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>

void i2c_setup();
void i2c_set_clock(uint32_t frequency);
void i2c_set_device(uint8_t i2c_device);
int i2c_write(uint8_t* buffer, uint16_t length);
int i2c_read(uint8_t* buffer, uint16_t length);

#endif