/*
 * Copyright (c) 2016-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,

 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== main_freertos.c ========
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* RTOS header files */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "bsp_gpio.h"
#include "misc.h"
#include "vs1053.h"
#include "bsp_uart.h"
#include "mp3.h"
#include "bsp_dma.h"
/* Driver header files */
// #include <ti/drivers/Board.h>
// #include <ti/drivers/GPIO.h>

// extern void temperatureThread(void *arg0);
// extern void consoleThread(void *arg0);

/* Stack size in 16-bit words */
#define THREADSTACKSIZE 768 / sizeof(portSTACK_TYPE)

void AppTaskCreate(void);
void Board_init(void);
void LED_Task(void);
void MP3_Play_Task(void);
void uart_Task(void);
TaskHandle_t AppTaskCreate_Handle_t = NULL;
TaskHandle_t LED_Task_Handle_t = NULL;
TaskHandle_t MP3_Task_Handle_t = NULL;
TaskHandle_t uart_Task_Handle_t = NULL;
SemaphoreHandle_t Binary_uart_Sem_Handle = NULL;


extern char usart1_Rx_Buf[USART1_READ_BUFF_SIZE];
uint16_t mp3_test[16] = {0};
uint8_t single_use = 0;

/*
 *  ======== main ========
 */

//fclk (SYSCLK) 72M //AHB(HCLK = SYSCLK)= 72M //APB2 (PCLK2 = HCLK)= 72M //APB1 (PCLK1 = HCLK/2)= 36M 
int main(void)
{
	BaseType_t xReturn = pdPASS;

	Board_init();

	xReturn = xTaskCreate((TaskFunction_t	) AppTaskCreate,	// pvTaskCode
					(const char *		)"AppTaskCreate",	// pcName
					(uint16_t			)128,				// usStackDepth
					(void *				)NULL,				// pvParameters
					(UBaseType_t		)3,					// uxPriority
					(TaskHandle_t *		)&AppTaskCreate_Handle_t); // pxCreatedTask
	if (xReturn != pdPASS)
	{
		/* xTaskCreate() failed */
		while (1) {}
	}

	/* Initialize the GPIO since multiple threads are using it */
	// GPIO_init();

	/* Start the FreeRTOS scheduler */
	vTaskStartScheduler();

	return (0);
}

void Board_init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	gpio_init();
	// GPIO_SetBits(GPIOB,GPIO_Pin_5);
	vs1053_init();
	
	// uart1_init(115200);
	// uart1_init(128000);
	// uart1_init(230400);
	uart1_init(256000);
	// uart1_init(460800);
	// uart1_init(500000);
#if (UART_Feedback_EN == 1)
	uart1_DMA1_init();
#endif
	// spi_DMA_init();
	uart1_DMA1_init();
}

void AppTaskCreate(void)
{
BaseType_t xReturn = pdPASS;

taskENTER_CRITICAL();
Binary_uart_Sem_Handle = xSemaphoreCreateBinary();
if(NULL != Binary_uart_Sem_Handle)
	printf("Binary_uart_Sem_Handle create successful!\n");
else
	while (1) {}

xReturn = xTaskCreate((TaskFunction_t	) LED_Task,	// pvTaskCode
				(const char *		)"LED_Task",	// pcName
				(uint16_t			)128,				// usStackDepth
				(void *				)NULL,				// pvParameters
				(UBaseType_t		)2,					// uxPriority
				(TaskHandle_t *		)&LED_Task_Handle_t); // pxCreatedTask
if (xReturn != pdPASS)
{
	/* xTaskCreate() failed */
	while (1) {}
}


xReturn = xTaskCreate((TaskFunction_t	) MP3_Play_Task,	// pvTaskCode
				(const char *		)"MP3_Play_Task",	// pcName
				(uint16_t			)128,				// usStackDepth
				(void *				)NULL,				// pvParameters
				(UBaseType_t		)3,					// uxPriority
				(TaskHandle_t *		)&MP3_Task_Handle_t); // pxCreatedTask
if (xReturn != pdPASS)
{
	/* xTaskCreate() failed */
	while (1) {}
}

// // send the received data to sender
// xReturn = xTaskCreate((TaskFunction_t	) uart_Task,	// pvTaskCode
// 				(const char *		)"uart_Task",	// pcName
// 				(uint16_t			)128,				// usStackDepth
// 				(void *				)NULL,				// pvParameters
// 				(UBaseType_t		)4,					// uxPriority
// 				(TaskHandle_t *		)&uart_Task_Handle_t); // pxCreatedTask
// if (xReturn != pdPASS)
// {
// 	/* xTaskCreate() failed */
// 	while (1) {}
// }


	vTaskDelete(AppTaskCreate_Handle_t);

	taskEXIT_CRITICAL();
}


void LED_Task(void)
{
	while (1)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_5);
		vTaskDelay(700);
		GPIO_ResetBits(GPIOB,GPIO_Pin_5);

		GPIO_SetBits(GPIOE,GPIO_Pin_5);
		vTaskDelay(700);
		GPIO_ResetBits(GPIOE,GPIO_Pin_5);
	}
}


	uint16_t i = 0;
	uint16_t j = 0;
	uint16_t k = 0;
	uint16_t r_index = 0;
	uint32_t finish_send_cntr=0;
void MP3_Play_Task(void)
{
	BaseType_t xReturn = pdPASS;
	uint8_t reset_flag = 0;
	uint8_t retry = 0;
	uint32_t num = 0;
	uint16_t end_fill_byte = 0;
	uint16_t temp = 0;
	uint8_t buf_init[32];
	volatile uint8_t read_squeue = 0;

	if (VS1053_DREQ == 1)
		printf("vs1053 need init!\n");
	printf("hardware init start\n");
	reset_flag = vs1053_hardware_reset();
	if (reset_flag == 0)
		printf("hardware init successful!\n");
	
	vs1053_soft_reset();




	printf("setting volume!\n");
	// vs1053_write_cmd(SPI_VOL, 0x7f7f);	// low
	vs1053_write_cmd(SPI_VOL, 0x3f3f);	//middle
	vs1053_read_reg(SPI_VOL);

	// vs1053_write_cmd(SPI_WRAMADDR, 0x1e04);	//play speed
	// temp = vs1053_read_reg(SPI_WRAM);
	// printf("play speed: %d\n", temp);

	DMA_Cmd (DEBUG_DMAx,ENABLE);
	mp3_arry3[MP3_BUFF_NUM-1] = 0x0E;	//E for empty
	// mp3_arry4[MP3_BUFF_NUM-1] = 0x0E;	//E for empty
	// mp3_arry5[MP3_BUFF_NUM-1] = 0x0E;

	while(1)
	{
/* SPI send data */

	//获取二值信号量 xSemaphore,没获取到则一直等待
		// xReturn = xSemaphoreTake(Binary_uart_Sem_Handle,/* 二值信号量句柄 */
		// 							portMAX_DELAY); /* 等待时间 */
		// if(pdPASS == xReturn)
		{

#if 1

			if(mp3_arry3[MP3_BUFF_NUM-1] == 0xFF)
			{
				for (r_index = 0; r_index < MP3_BUFF_NUM-1; )
				{
					vs1053_send_music_data(&mp3_arry3[r_index], 32);	//uint: 2byte, 16*2
					r_index += 32;
				}
				mp3_arry3[r_index] = 0x0E;
			}

#elif 0
			switch (read_squeue)
			{
				case 0:
					if(mp3_arry3[MP3_BUFF_NUM-1] == 0xFF)
					{
						for (r_index = 0; r_index < MP3_BUFF_NUM-1; )
						{
							vs1053_send_music_data(&mp3_arry3[r_index], 32);	//uint: 2byte, 16*2
							r_index += 32;
						}

						mp3_arry3[r_index] = 0x0E;
						read_squeue = 1;
					}
					
					{
						// if(finish_send_cntr++>=6999999)
						// {
						// 	for (i = 0; i<=MP3_BUFF_NUM-1; i++)
						// 	{
						// 		printf("0x%02x  ", mp3_arry3[i]);
						// 		if(i%6 ==0)
						// 		{
						// 			printf("  3:\n");
						// 		}
						// 	}
						// 	printf("\n");
						// 	for (i = 0; i<=MP3_BUFF_NUM-1; i++)
						// 	{
						// 		printf("0x%02x  ", mp3_arry4[i]);
						// 		if(i%6 ==0)
						// 		{
						// 			printf("  4:\n");
						// 		}
						// 	}
						// 	printf("\n");
						// 	for (i = 0; i<=MP3_BUFF_NUM-1; i++)
						// 	{
						// 		printf("0x%02x  ", mp3_arry5[i]);
						// 		if(i%6 ==0)
						// 		{
						// 			printf("  5:\n");
						// 		}
						// 	}
						// 	printf("\n");

						// 	while(1);
						// }
					}

					break;

				case 1:
					if(mp3_arry4[MP3_BUFF_NUM-1] == 0xFF)
					{
						for (r_index = 0; r_index < MP3_BUFF_NUM-1; )
						{
							vs1053_send_music_data(&mp3_arry4[r_index], 32);	//uint: 2byte, 16*2
							r_index += 32;
						}
						
						mp3_arry4[r_index] = 0x0E;
						read_squeue = 0;
					}
					break;

				// case 2:
				// 	if(mp3_arry5[MP3_BUFF_NUM-1] == 0xFF)
				// 	{
				// 		for (r_index = 0; r_index < MP3_BUFF_NUM-1; )
				// 		{
				// 			vs1053_send_music_data(&mp3_arry5[r_index], 32);	//uint: 2byte, 16*2
				// 			r_index += 32;
				// 		}

				// 		mp3_arry5[r_index] = 0x0E;
				// 		read_squeue = 0;
				// 	}

				// 	break;
				default:
					break;
			}
#endif
		}

		// vTaskDelay(1);
		// {
			if (finish_send_cntr++ == 6999999)
			{
#if 1
				vs1053b_stop();

#else

				printf("mp3_arry4:\n");
				for (i = 0; i < MP3_BUFF_NUM; i++)
				{
					printf("0x%02x  ", mp3_arry4[i]);
					if(i%8 ==0)
						printf("\n");
				}
				
				printf("\n");
				printf("\n");
				printf("mp3_arry5:\n");
				for (i = 0; i < MP3_BUFF_NUM; i++)
				{
					printf("0x%02x  ", mp3_arry5[i]);
					if(i%8 ==0)
						printf("\n");
				}
#endif
			}
		// }
/* end of SPI send data */


	}



	/*SPI Sine test*/
	// vs1053_write_cmd(SPI_AUDATA,0xBB81);

	// vs1053_write_cmd(SPI_MODE, 0x0820);
	// vs1053_read_reg(SPI_MODE);
	// vs1053_send_music_data(&mp3_arry2[0],4);
	/*SPI Sine test end*/


	/*Sine test*/
#if 0
	VS1053_xDCS(1);
	VS1053_xCS(0);
	vs1053_read_write_byte(0x02);	//write cmd
	vs1053_read_write_byte(SPI_MODE);
	vs1053_read_write_byte(0x08);	// soft reset
	vs1053_read_write_byte(0x20);
	VS1053_xCS(1);
	vTaskDelay(3);  //test //vTaskDelay(3);

	VS1053_xCS(0);
	vs1053_read_write_byte(0x02);	//write cmd
	vs1053_read_write_byte(SPI_VOL);
	vs1053_read_write_byte(0x7F);	// soft reset
	vs1053_read_write_byte(0x7F);
	VS1053_xCS(1);
	vTaskDelay(3);  //test //vTaskDelay(3);


	VS1053_xDCS(0);
	vs1053_read_write_byte(0x53);	//write cmd
	vs1053_read_write_byte(0xEF);
	vs1053_read_write_byte(0x6E);	// soft reset
	vs1053_read_write_byte(0x7E);
	vs1053_read_write_byte(0x00);	//write cmd
	vs1053_read_write_byte(0x00);
	vs1053_read_write_byte(0x00);	// soft reset
	vs1053_read_write_byte(0x00);
	VS1053_xDCS(1);
#endif
	/*Sine test stop*/


	// vs1053b_stop();
#if 0
	{
		num = 0;
		// printf("mp3 file num in total: [%d]\n",(sizeof(mp3_arry2)));

		do
		{
			/* last piece*/
			if (num+32 >= 165032 )	//sizeof is 2* (mp3_arry len)
			{
				// vs1053_send_music_data(&mp3_arry[num], (sizeof(mp3_arry)>>1) - (num<<4));
				printf("finish!\n");
				break;
			}
			/* last piece*/

			vs1053_send_music_data(&mp3_arry[num],32);
			// printf("send [%d]\n",num);
			num += 32;
		} while (1);
		
		printf("send mp3 finished!\n");
		while(1);
#if 0
		vs1053_write_cmd(SPI_WRAMADDR, 0x1e06);
		temp = vs1053_read_reg(SPI_WRAM);
		for (i = 0; i < 65; i++)
		{
			memset(buf_init, temp & 0xFF, sizeof(uint8_t) * 32);           /* init */
			vs1053b_dat_write(buf_init, 32);
		}

		temp = vs1053_read_reg(SPI_MODE);
		temp |= 1 << 3;						/* set the SM_CANCEL */
		vs1053_write_cmd(SPI_MODE, temp);

		for (i = 0; i < 64; i++)
		{
			end_fill_byte_send(0x00, 1);
			temp = vs1053_read_reg(SPI_MODE);
			if ((temp & (1 << 3)) == 0)
			{
				break;
			}
		}


		temp = vs1053_read_reg(SPI_HDAT0);
		if (temp != 0)
		{
			printf("dat0 and dat1 ara invalid.\n");
		}
		temp = vs1053_read_reg(SPI_HDAT1);
		if (temp != 0)
		{
			printf("dat0 and dat1 ara invalid.\n");
		}
#elif 0
		vs1053_write_cmd(SPI_WRAMADDR, 0x1e06);
		end_fill_byte = vs1053_read_reg(SPI_WRAM);
		end_fill_byte_send(end_fill_byte, 65);
		vs1053_write_cmd(SPI_MODE, 0x0808);

		end_fill_byte = vs1053_read_reg(SPI_WRAM);
		do
		{
			end_fill_byte_send(end_fill_byte, 2);
			vTaskDelay(1500);
			// vs1053_send_music_data(mp3_test, 16);
			
			temp = vs1053_read_reg(SPI_MODE);

			vs1053_read_reg(SPI_VOL);	//test

		} while (temp != 0x0800);
#endif

	}
#endif


	// vs1053_mode = vs1053_read_reg(SPI_MODE);
	// vs1053_send_music_data(music_data, MUSIC_DATA_LEN);
}

#if (UART_Feedback_EN == 1)
void uart_Task(void)
{
	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
	while (1)
	{
	//获取二值信号量 xSemaphore,没获取到则一直等待
		xReturn = xSemaphoreTake(Binary_uart_Sem_Handle,/* 二值信号量句柄 */
									portMAX_DELAY); /* 等待时间 */
		if(pdPASS == xReturn)
		{
			printf("received message: %s\n",usart1_Rx_Buf);
			memset(usart1_Rx_Buf,0,USART1_READ_BUFF_SIZE);/* 清零 */
		}
	}
}
#endif

#if (UART_Feedback_EN == 0)
void uart_Task(void)
{
		BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
		// uint8_t fill_squeue = 0;

		while(1)
		{
			vTaskDelay(1000);
		}

		while (1)
		
		{
		//获取二值信号量 xSemaphore,没获取到则一直等待
			xReturn = xSemaphoreTake(Binary_uart_Sem_Handle,/* 二值信号量句柄 */
										portMAX_DELAY); /* 等待时间 */
			if(pdPASS == xReturn)
			{
#if 0
/* UART received data */
				for (i = 0; i < DMA_BUFF_NUM; i++)
				{
					switch (fill_squeue)
					{
						case 0:
							if(mp3_arry4[MP3_BUFF_NUM-1] == 0x0E)	// Some space is still available, and other buff not ready begin filling
							{
								mp3_arry4[j++] = mp3_arry3[i];
								if (j == MP3_BUFF_NUM - 1)
								{
									mp3_arry4[j] = 0xFF;	// F for full
									fill_squeue = 1;
								}
							}
							else
							{
								printf("error: DMA data is full!!");
								while(1);
							}
							break;

						case 1:
							if(mp3_arry5[MP3_BUFF_NUM-1] == 0x0E)
							{
								mp3_arry5[k++] = mp3_arry3[i];
								if(k == MP3_BUFF_NUM - 1)
								{
									mp3_arry5[k] = 0xFF;	// F for full
									fill_squeue = 0;
								}
							}
							else
							{
								printf("error: DMA data is full!!");
								while(1);
							}
							break;
						default:
							break;
					}
				}

				if( (j == MP3_BUFF_NUM - 1) && (mp3_arry4[MP3_BUFF_NUM-1] == 0x0E) )
					j = 0;
				else if( (k == MP3_BUFF_NUM - 1) && (mp3_arry5[MP3_BUFF_NUM-1] == 0x0E) )
					k = 0;
/** end of UART received data **/
#endif	
			}
		}


#endif
}

