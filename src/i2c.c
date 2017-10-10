#include <systimer.h>
#include <gpio.h>

#include "i2c.h"

// https://github.com/alanbarr/RaspberryPi-GPIO/blob/master/src/i2c.c

#define BSC0_BASE (volatile uint32_t *)(0x3F205000) // 0x20205000 pour rpi1
#define BSC1_BASE (volatile uint32_t *)(0x3F804000) // 0x20804000 pour rpi1

#define I2C0_SDA_PIN        0
#define I2C0_SCL_PIN        1

// Camera pin are 28&27 those are muxed with 0&1
// https://www.raspberrypi.org/forums/viewtopic.php?f=44&t=137848#p917132
#define I2C0_CAM_SDA_PIN    28
#define I2C0_CAM_SCL_PIN    29

#define I2C1_SDA_PIN        2
#define I2C1_SCL_PIN        3

#define BSC0_C          *(BSC0_BASE + 0x00)
#define BSC0_S          *(BSC0_BASE + 0x01)
#define BSC0_DLEN       *(BSC0_BASE + 0x02)
#define BSC0_A          *(BSC0_BASE + 0x03)
#define BSC0_FIFO       *(BSC0_BASE + 0x04)
#define BSC0_DIV        *(BSC0_BASE + 0x05)
#define BSC0_DEL        *(BSC0_BASE + 0x06)

#define BSC1_C          *(BSC1_BASE + 0x00)
#define BSC1_S          *(BSC1_BASE + 0x01)
#define BSC1_DLEN       *(BSC1_BASE + 0x02)
#define BSC1_A          *(BSC1_BASE + 0x03)
#define BSC1_FIFO       *(BSC1_BASE + 0x04)
#define BSC1_DIV        *(BSC1_BASE + 0x05)
#define BSC1_DEL        *(BSC1_BASE + 0x06)

#define BSC_I2CEN     (1 << 15)
#define BSC_INTR      (1 << 10)
#define BSC_INTT      (1 << 9)
#define BSC_INTD      (1 << 8)
#define BSC_ST        (1 << 7)
#define BSC_CLEAR     (1 << 4)
#define BSC_READ      1

#define START_READ      BSC_I2CEN | BSC_ST | BSC_CLEAR | BSC_READ
#define START_WRITE     BSC_I2CEN | BSC_ST

#define BSC_CLKT      (1 << 9)
#define BSC_ERR       (1 << 8)
#define BSC_RXF       (1 << 7)
#define BSC_TXE       (1 << 6)
#define BSC_RXD       (1 << 5)
#define BSC_TXD       (1 << 4)
#define BSC_RXR       (1 << 3)
#define BSC_TXW       (1 << 2)
#define BSC_DONE      (1 << 1)
#define BSC_TA        1

#define CLEAR_STATUS    BSC_CLKT | BSC_ERR | BSC_DONE

#define PAGESIZE        4096
#define BLOCK_SIZE      4096

#define CORE_CLK_HZ 250000000

#define I2C_SDA_PIN     I2C0_CAM_SDA_PIN
#define I2C_SCL_PIN     I2C0_CAM_SCL_PIN
#define I2C_A           BSC0_A
#define I2C_C           BSC0_C
#define I2C_S           BSC0_S
#define I2C_DIV         BSC0_DIV
#define I2C_DEL         BSC0_DEL
#define I2C_FIFO        BSC0_FIFO
#define I2C_DLEN        BSC0_DLEN

void i2c_setup() {
    // Disable internal resistors
    gpio_set_resistor(I2C_SDA_PIN, PULL_DISABLE);
    gpio_set_resistor(I2C_SCL_PIN, PULL_DISABLE);
    
    // Set I2C pins to ALT_0 mode
    gpio_set_mode(I2C_SDA_PIN, ALT_0);
    gpio_set_mode(I2C_SCL_PIN, ALT_0);
    
    i2c_set_clock(100000);
    
    // Setup Control Register, Enable BSC Controller, Clear the FIFO
    I2C_C = BSC_I2CEN | BSC_CLEAR; 

    // Setup Status Register, Clear NACK ERR flag, Clear Clock stretch flag,
    // Clear Done flag.
    I2C_S = BSC_ERR | BSC_CLKT | BSC_DONE;
}

void i2c_set_clock(uint32_t frequency) {
    I2C_DIV = CORE_CLK_HZ / frequency;
}

void i2c_set_device(uint8_t i2c_device) {
    I2C_A = i2c_device;
    delay_us(200);
}

int i2c_write(uint8_t* buffer, uint16_t length) {
    int rtn = 0;
    uint16_t dataIndex = 0;
    uint16_t dataRemaining = length;

    /* Clear the FIFO */
    I2C_C |= BSC_CLEAR;

    /* Configure Control for a write */
    I2C_C &= ~BSC_READ;

    /* Set the Data Length register to dataLength */
    I2C_DLEN = length;

    /* Configure Control Register for a Start */
    I2C_C |= BSC_ST;

    /* Main transmit Loop - While Not Done */
    while (!(I2C_S & BSC_DONE)) {
        while ((I2C_S & BSC_TXD) && dataRemaining) {
            I2C_FIFO = buffer[dataIndex];
            dataIndex++;
            dataRemaining--;
        }

        delay_us(200);
    }

    /* Received a NACK */
    if (I2C_S & BSC_ERR) {
        I2C_S |= BSC_ERR;
        //dbgPrint(DBG_INFO, "Received a NACK.");
        rtn = 1;
    } else if (I2C_S & BSC_CLKT) { // Received Clock Timeout error
        I2C_S |= BSC_CLKT;
        //dbgPrint(DBG_INFO, "Received a Clock Stretch Timeout.");
        rtn = 2;
    } else if (dataRemaining) {
        //dbgPrint(DBG_INFO, "BSC signaled done but %d data remained.", dataRemaining);
        rtn = 4;
    }

    /* Clear the DONE flag */
    I2C_S |= BSC_DONE;

    return rtn;
}

int i2c_read(uint8_t* buffer, uint16_t length) {
    int rtn = 0;
    uint16_t bufferIndex = 0;
    uint16_t dataRemaining = length;

     /* Clear the FIFO */
    I2C_C |= BSC_CLEAR;

    /* Configure Control for a write */
    I2C_C |= BSC_READ;

    /* Set the Data Length register to dataLength */
    I2C_DLEN = length;

    /* Configure Control Register for a Start */
    I2C_C |= BSC_ST;

    /* Main Receive Loop - While Transfer is not done */
    while (!(I2C_S & BSC_DONE)) {
        /* FIFO Contains Data. Read until empty */
        while ((I2C_S & BSC_RXD) && dataRemaining) {
            buffer[bufferIndex] = I2C_FIFO;
            bufferIndex++;
            dataRemaining--;
        }

        delay_us(200);
    }

    /* FIFO Contains Data. Read until empty */
    while ((I2C_S & BSC_RXD) && dataRemaining) {
        buffer[bufferIndex] = I2C_FIFO;
        bufferIndex++;
        dataRemaining--;
    }

    if (I2C_S & BSC_ERR) { // Received a NACK
        I2C_S |= BSC_ERR;
        //dbgPrint(DBG_INFO, "Received a NACK");
        rtn = 1;
    } else if (I2C_S & BSC_CLKT) { // Received Clock Timeout error.
        I2C_S |= BSC_CLKT;
        //dbgPrint(DBG_INFO, "Received a Clock Stretch Timeout");
        rtn = 2;
    } else if (dataRemaining) {
        //dbgPrint(DBG_INFO, "BSC signaled done but data remained.");
        rtn = 4;
    }

    /* Clear the DONE flag */
    I2C_S |= BSC_DONE;
    
    return rtn;
}