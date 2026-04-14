#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include <misc.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "bsp_uart.h"
#include "bsp_dma.h"
#include "mp3.h"
#include "config.h"


uint8_t usart1_Rx_Buf[USART1_READ_BUFF_SIZE];

#if (UART_Feedback_EN == 1)
void uart1_DMA1_init(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	// 开启DMA时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	// 设置DMA源地址：串口数据寄存器地址*/
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&DEBUG_USARTx->DR);
	// 内存地址(要传输的变量的指针)
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)usart1_Rx_Buf;
	// 方向：从外设到内存
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	// 传输大小	
	DMA_InitStructure.DMA_BufferSize = USART1_READ_BUFF_SIZE;
	// 外设地址不增
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	// 内存地址自增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	// 外设数据单位	
	DMA_InitStructure.DMA_PeripheralDataSize = 	DMA_PeripheralDataSize_Byte;
	// 内存数据单位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	// DMA模式，一次或者循环模式
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	// DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	
	// 优先级
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	// 禁止内存到内存的传输
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	// 配置DMA通道
	DMA_Init(DEBUG_DMAx, &DMA_InitStructure);
	// 清除DMA所有标志
	DMA_ClearFlag(DMA1_FLAG_TC5);
	DMA_ITConfig(DEBUG_DMAx, DMA_IT_TE, ENABLE);
	// 使能DMA
	DMA_Cmd (DEBUG_DMAx,ENABLE);
}
#endif

extern SemaphoreHandle_t Binary_uart_Sem_Handle;
extern uint32_t finish_send_cntr;
volatile uint8_t fill_squeue = 1;
//DMA_Mode_Normal, not DMA_Mode_Circular then need renew function.
void uart1_DMA1_renew(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	BaseType_t pxHigherPriorityTaskWoken;
	BaseType_t xReturn = pdPASS;
	// // 关闭DMA ，防止干扰
	// DMA_Cmd(DEBUG_DMAx, DISABLE);
	// // 清DMA标志位
	// DMA_ClearFlag(DMA1_FLAG_TC5);
	// //  重新赋值计数值，必须大于等于最大可能接收到的数据帧数目
	// DEBUG_DMAx->CNDTR = USART1_READ_BUFF_SIZE;
	// DMA_Cmd(DEBUG_DMAx, ENABLE);
	// /* 
	// xSemaphoreGiveFromISR(SemaphoreHandle_t xSemaphore,
	// 											BaseType_t *pxHigherPriorityTaskWoken);
	// */

	finish_send_cntr = 0;
// finish_send_cntr ++;

#if 1
	DEBUG_DMAx->CMAR = (uint32_t)mp3_arry3;
	mp3_arry3[MP3_BUFF_NUM - 1] = 0xFF;	// F for full
#elif 0
	// 关闭DMA ，防止干扰
	DMA_Cmd(DEBUG_DMAx, DISABLE);

/* DMA change channel */

	switch (fill_squeue)
	{
		case 0:
			if(mp3_arry3[MP3_BUFF_NUM-1] == 0x0E)	// Some space is still available, and other buff not ready begin filling
			{
				DEBUG_DMAx->CMAR = (uint32_t)mp3_arry3;
				
				mp3_arry4[MP3_BUFF_NUM - 1] = 0xFF;	// F for full
				fill_squeue = 1;
			}
			else
			{
				printf("arry3 error!!\narry3: 0x%02x, arry4: 0x%02x, arry5: 0x%02x error: DMA data is full!!",mp3_arry3[MP3_BUFF_NUM-1], mp3_arry4[MP3_BUFF_NUM-1]);
				while(1);
			}
			break;

		case 1:
			if(mp3_arry4[MP3_BUFF_NUM-1] == 0x0E)
			{
				DEBUG_DMAx->CMAR = (uint32_t)mp3_arry4;

				mp3_arry3[MP3_BUFF_NUM - 1] = 0xFF;	// F for full
				fill_squeue = 0;
			}
			else
			{
				printf("arry4 error!!\narry3: 0x%02x, arry4: 0x%02x, arry5: 0x%02x error: DMA data is full!!",mp3_arry3[MP3_BUFF_NUM-1], mp3_arry4[MP3_BUFF_NUM-1]);
				while(1);
			}
			break;

		// case 2:
		// 	if(mp3_arry5[MP3_BUFF_NUM-1] == 0x0E)
		// 	{
		// 		DEBUG_DMAx->CMAR = (uint32_t)mp3_arry5;

		// 		mp3_arry4[MP3_BUFF_NUM - 1] = 0xFF;	// F for full
		// 		fill_squeue = 0;
		// 	}
		// 	else
		// 	{
		// 		printf("arry5 error!!\narry3: 0x%02x, arry4: 0x%02x, arry5: 0x%02x error: DMA data is full!!",mp3_arry3[MP3_BUFF_NUM-1], mp3_arry4[MP3_BUFF_NUM-1], mp3_arry5[MP3_BUFF_NUM-1]);
		// 		while(1);
		// 	}
		// 	break;

		default:
			break;
	}
/** end of DMA change channel **/

	//  重新赋值计数值，必须大于等于最大可能接收到的数据帧数目
	DEBUG_DMAx->CNDTR = USART1_READ_BUFF_SIZE;
	DMA_Cmd(DEBUG_DMAx, ENABLE);
	/* 
	xSemaphoreGiveFromISR(SemaphoreHandle_t xSemaphore,
												BaseType_t *pxHigherPriorityTaskWoken);
	*/
#endif


	//给出二值信号量 ，发送接收到新数据标志，供前台程序查询
	// xReturn = xSemaphoreGiveFromISR(Binary_uart_Sem_Handle,&pxHigherPriorityTaskWoken);	//释放二值信号量
	// if(xReturn == errQUEUE_FULL)
	// {
	// 	printf("error: UART too fast or DMA buff size is fewer!!");
	// }

	//如果需要的话进行一次任务切换，系统会判断是否需要进行切换
	// portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);

	/* 
	DMA 开启，等待数据。注意，如果中断发送数据帧的速率很快，MCU来不及处理此次接收到的数据，
	中断又发来数据的话，这里不能开启，否则数据会被覆盖。有2种方式解决：

	1. 在重新开启接收DMA通道之前，将LumMod_Rx_Buf缓冲区里面的数据复制到另外一个数组中，
	然后再开启DMA，然后马上处理复制出来的数据。

	2. 建立双缓冲，在LumMod_Uart_DMA_Rx_Data函数中，重新配置DMA_MemoryBaseAddr 的缓冲区地址，
	那么下次接收到的数据就会保存到新的缓冲区中，不至于被覆盖。
	*/
}


/* uart_RX to Memory  （DMA）*/
void uart1_DMA1_init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	// 开启DMA时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	// 设置DMA源地址：串口数据寄存器地址*/
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&DEBUG_USARTx->DR);
	// 内存地址(要传输的变量的指针)
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)mp3_arry3;
	// 方向：从外设到内存
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	// 传输大小	
	DMA_InitStructure.DMA_BufferSize = USART1_READ_BUFF_SIZE;
	// 外设地址不增
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	// 内存地址自增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	// 外设数据单位	
	DMA_InitStructure.DMA_PeripheralDataSize = 	DMA_PeripheralDataSize_Byte;
	// 内存数据单位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	// DMA模式，一次或者循环模式
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	// DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	
	// 优先级
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	// 禁止内存到内存的传输
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	// 配置DMA通道
	DMA_Init(DEBUG_DMAx, &DMA_InitStructure);
	// 清除DMA所有标志

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = DEBUG_DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=6 ;//抢占优先级,分组4:有16位 抢占优先级,0位子优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	DMA_ClearFlag(DEBUG_DMA_FALG_TC);
	DMA_ITConfig(DEBUG_DMAx, DMA_IT_TC, ENABLE);	// TC: transfer complete
	// 使能DMA

}



/* end of uart_RX to Memory  （DMA）*/
