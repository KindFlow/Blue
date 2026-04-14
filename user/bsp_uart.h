#ifndef _BSP_UART_H
#define _BSP_UART_H

#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"

#define DEBUG_USARTx					USART1
#define DEBUG_USART_IRQ					USART1_IRQn
#define DEBUG_USART_IRQHandler			USART1_IRQHandler

#define USART1_READ_BUFF_SIZE			54272//27008	//18048

void uart1_init(uint32_t bound);


void uart_send_byte( USART_TypeDef * pUSARTx, uint8_t ch);
void uart_send_string( USART_TypeDef * pUSARTx, char *str);



#endif /* "bsp_uart.h" _BSP_UART_H */
