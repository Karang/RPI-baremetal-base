#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <gpio.h>
#include <i2c.h>
#include <uart.h>
#include <gpu.h>
#include <systimer.h>

#define LED_ACT 47

#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT   768
#define SCREEN_BPP      16

void print_info_header() {
    printf("Raspberry-pi base for baremetal experimentations.\r\n");
	printf("Author: Arthur Hennequin\r\n\r\n");
	
	mailbox_property_t* mp;
	
	mailbox_property_init();
    mailbox_property_add_tag(TAG_GET_BOARD_MODEL);
    mailbox_property_add_tag(TAG_GET_BOARD_REVISION);
    mailbox_property_add_tag(TAG_GET_FIRMWARE_VERSION);
    mailbox_property_add_tag(TAG_GET_BOARD_MAC_ADDRESS);
    mailbox_property_add_tag(TAG_GET_BOARD_SERIAL);
    mailbox_property_add_tag(TAG_GET_MAX_CLOCK_RATE, TAG_CLOCK_ARM);
    mailbox_property_process();
   
    mp = mailbox_get_property(TAG_GET_BOARD_MODEL);
    if (mp) printf("Board Model: %lu\r\n", mp->data.value_32);
    else printf("Board Model: NULL\r\n");

    mp = mailbox_get_property(TAG_GET_BOARD_REVISION);
    if (mp) printf("Board Revision: %lu\r\n", mp->data.value_32);
    else printf("Board Revision: NULL\r\n");
    
    mp = mailbox_get_property(TAG_GET_FIRMWARE_VERSION);
    if (mp) printf("Firmware Version: %lu\r\n", mp->data.value_32);
    else printf("Firmware Version: NULL\r\n");

    mp = mailbox_get_property(TAG_GET_BOARD_MAC_ADDRESS);
    if (mp) printf("MAC Address: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\r\n",
            mp->data.buffer_8[0], mp->data.buffer_8[1], mp->data.buffer_8[2],
            mp->data.buffer_8[3], mp->data.buffer_8[4], mp->data.buffer_8[5]);
    else printf("MAC Address: NULL\r\n");

    mp = mailbox_get_property(TAG_GET_BOARD_SERIAL);

    if (mp) printf("Serial Number: %8.8X%8.8X\r\n",
            mp->data.buffer_32[0], mp->data.buffer_32[1]);
    else printf("Serial Number: NULL\r\n");

    mp = mailbox_get_property(TAG_GET_MAX_CLOCK_RATE);
    if (mp) printf("Maximum ARM Clock Rate: %luHz\r\n", mp->data.buffer_32[1]);
    else printf("Maximum ARM Clock Rate: NULL\r\n");
	
	printf("Framebuffer width=%d, height=%d, bpp=%d\r\n", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);
}

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    (void) r0;
	(void) r1;
	(void) atags;
 
    gpio_set_mode(LED_ACT, OUTPUT);
    
    // Init uart and print infos
    uart_init();
	print_info_header();
    
    // Checkerboard pattern
    uint16_t* screen = gpu_init(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);
    for (int y=0 ; y<SCREEN_HEIGHT ; y++) {
        for (int x=0 ; x<SCREEN_WIDTH ; x++) {
            int a = (y / 50) % 2;
            int b = (x / 50) % 2;
            if ((a^b) == 1)
                screen[y*SCREEN_WIDTH+x] = 0xFFFF;
            else
                screen[y*SCREEN_WIDTH+x] = 0x0000;
        }
    }
    
    // Blink led forever
    for (;;) {
        gpio_write(LED_ACT, HIGH);
        delay_ms(500);
        gpio_write(LED_ACT, LOW);
        delay_ms(500);
    }
}