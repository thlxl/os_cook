#ifndef STM32F10_SERIAL_H
#define STM32F10_SERIAL_H

#include <stdio.h>
#include "stm32f10_registers.h"

void USART_Init(void);
void USART_Transmit(char data);
int fputc(int ch, FILE *f);

#endif
