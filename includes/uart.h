#ifndef __UART_H__
#define __UART_H__

#include <stddef.h>
#include <stdint.h>

void uart_init();
void uart_putc(unsigned char byte);
unsigned char uart_getc();

#endif