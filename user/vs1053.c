#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_dma.h"

#include "vs1053.h"

#include "FreeRTOS.h"
#include <stdio.h>
#include "task.h"

uint16_t music_data[MUSIC_DATA_LEN] = {0};


static __IO uint32_t  SPITimeout = SPIT_LONG_TIMEOUT;


void vs1053_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD, ENABLE );//PORTB时钟使能 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;  // PB12 推挽
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);	// xCS

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;  // PD11 推挽 
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_SetBits(GPIOD,GPIO_Pin_11);	// xDCS

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;  // PD13 推挽 
	GPIO_Init(GPIOD, &GPIO_InitStructure);	//xRESET


	/****test code start******/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;  //test  PB13 推挽 
	GPIO_Init(GPIOB, &GPIO_InitStructure);	//test   clk

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;  //test  PB15 推挽 
	GPIO_Init(GPIOB, &GPIO_InitStructure);	//test   MOSI

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;  //test  PB14 推挽 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);	//test   MISO
	/*test code end*/

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	//PD12
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);	// DREQ

	VS1053_xCS(1);
	VS1053_xDCS(1); // test
	VS1053_xRESET(1);
}

void spi2_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//PORTB时钟使能
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2,  ENABLE );//SPI2时钟使能

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; // PB13:SCK; PB14:MISO; PB15:MOSI
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	//设置SPI的数据大小:SPI发送接收16位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;			// 串行同步时钟的空闲状态电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;		// 串行同步时钟的第1 or 第2个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;// SPI_BaudRatePrescaler_8;		//定义波特率预分频的值:波特率预分频值为8, 36M/8=4.5M
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	// SPI_InitStructure.SPI_CRCPolynomial = 0x8005;	//CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

	SPI_Cmd(SPI2, ENABLE); //使能SPI外设
	// SPI_I2S_SendData(SPI2,0xFF);
}

// void spi_DMA_init(void)
// {
// 	DMA_InitTypeDef DMA_InitStructure;

// 	// 开启DMA时钟
// 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
// 	// 设置DMA源地址：串口数据寄存器地址*/
// 	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SPI2_SLAVE_DR_Base;
// 	// 内存地址(要传输的变量的指针)
// 	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)mp3_arry;
// 	// 方向：从外设到内存
// 	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
// 	// 传输大小
// 	DMA_InitStructure.DMA_BufferSize = SPI2_READ_BUFF_SIZE;
// 	// 外设地址不增
// 	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
// 	// 内存地址自增
// 	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
// 	// 外设数据单位	
// 	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
// 	// 内存数据单位
// 	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	 
// 	// DMA模式，一次或者循环模式
// 	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;
// 	// DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	
// 	// 优先级：中
// 	DMA_InitStructure.DMA_Priority = DMA_Priority_Low; 
// 	// 禁止内存到内存的传输
// 	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
// 	// 配置DMA通道		   
// 	DMA_Init(DMA1_Channel2, &DMA_InitStructure);		
// 	// 清除DMA所有标志
// 	DMA_ClearFlag(DMA1_FLAG_TC5);
// 	DMA_ITConfig(DMA1_Channel2, DMA_IT_TE, ENABLE);
// 	// 使能DMA
// 	DMA_Cmd (DMA1_Channel2,ENABLE);
// }

static uint16_t SPI_TIMEOUT_UserCallback(uint8_t errorCode)
{
  /* 等待超时后的处理,输出错误信息 */
  printf("SPI overtime!errorCode = %d",errorCode);
  return 0;
}

uint8_t vs1053_hardware_reset(void)
{
	uint8_t retry = 0;
	VS1053_xRESET(0);
	vTaskDelay(20);
	VS1053_xDCS(1);
	VS1053_xCS(1);
	VS1053_xRESET(1);
	while(VS1053_DREQ == 0 && retry < 10)
	{
		vTaskDelay(10);
		retry++;
	}

	if (retry == 10)	//over time
	{
		return 1;
	}
	else				// success
		return 0;
}

void vs1053_soft_reset(void)
{

	uint8_t retry = 0;
	uint16_t receive_message = 0;
	uint8_t flag = 0;
	uint16_t clk;
	//uint8_t temp=0;
	while(VS1053_DREQ == 0 && retry < 10)
	{
		vTaskDelay(10);
		retry++;
	}
	if (retry == 10)
		{while(1){}}
	printf("software init start\n");
	do
	{
{
		// VS1053_xDCS(1);
		// VS1053_xCS(0);
		// vs1053_read_write_byte(0x02);	//write cmd
		// vs1053_read_write_byte(SPI_MODE);
		// vs1053_read_write_byte(0x08);	// soft reset
		// vs1053_read_write_byte(0x04);
		// VS1053_xCS(1);
		// vTaskDelay(3);  //test //vTaskDelay(3);


		// retry = 0;
		// while(VS1053_DREQ == 0 && retry < 10)
		// {
		// 	vTaskDelay(10);
		// 	retry++;
		// }
		// if (retry == 10)
		// 	{while(1){}}


		// VS1053_xDCS(1);
		// VS1053_xCS(0);
		// vs1053_read_write_byte(0x03); // read reg, need to be 0x0800, means reset successful.
		// vs1053_read_write_byte(SPI_MODE);
		// receive_message = vs1053_read_write_byte(0xff); // dummy write
		// receive_message = receive_message << 8;
		// receive_message += vs1053_read_write_byte(0xff); // dummy write
		// VS1053_xCS(1);
}
		spi2_init();
		vs1053_write_cmd(SPI_MODE, 0x0804);
		receive_message = vs1053_read_reg(SPI_MODE);

	} while (receive_message != 0x0800);
	printf("software init successful!\n");
	receive_message = 0;

	do
	{
		printf("setting clk multiply\n");
		retry=0;
		while(VS1053_DREQ == 0 && retry < 10)
		{
			vTaskDelay(10);
			retry++;
		}
		if (retry == 10)
			{while(1){}}

{
		// VS1053_xDCS(1);
		// VS1053_xCS(0);
		// vs1053_read_write_byte(0x02);	//write cmd
		// vs1053_read_write_byte(SPI_CLOCKF);
		// vs1053_read_write_byte(0x98);	// 3.5x CLK (12.288M*3.5), XTALI x3.5, SC_ADD 2.0x
		// vs1053_read_write_byte(0x00);
		// VS1053_xCS(1);
		// vTaskDelay(20);

		// retry = 0;
		// while(VS1053_DREQ == 0 && retry < 10)
		// {
		// 	vTaskDelay(10);
		// 	retry++;
		// }
		// if (retry == 10)
		// 	{while(1){}}


		// VS1053_xDCS(1);
		// VS1053_xCS(0);
		// vs1053_read_write_byte(0x03);	// read reg, 0x9800, means set successful.
		// vs1053_read_write_byte(SPI_CLOCKF);
		// receive_message = vs1053_read_write_byte(0xff);	//dummy write
		// receive_message = receive_message<<8;
		// receive_message += vs1053_read_write_byte(0xff);	//dummy write
		// VS1053_xCS(1);
}

		vs1053_write_cmd(SPI_CLOCKF, 0x9800);
		receive_message = vs1053_read_reg(SPI_CLOCKF);
		vTaskDelay(300);
	} while (0x9800 != receive_message );
	printf("clk multiply set successful!\n");

	// spi2_init();
	vs1053b_clock_frequency_convert_to_register(VS1053B_BASIC_DEFAULT_PLAY_CLOCK_FREQUENCY, &clk);
	vs1053b_set_clock_frequency(clk);

	printf("vs1053 all is ready!!\n");
	
}

uint8_t vs1053_send_music_data(uint8_t *p_data,uint16_t len)
{
	uint16_t i = 0;
	uint16_t temp;
	uint8_t retry;
	
	retry=0;
	while(VS1053_DREQ == 0 && retry < 10)
	{
		vTaskDelay(6);
		retry++;
	}
	if (retry == 20)
	{
		printf("1053 SPI ERROR!\n");
		while(1)
		{
			vTaskDelay(1000);
		}
	}
	// while(VS1053_DREQ ==0)
	// {;}

	VS1053_xDCS(0);

	temp = SPI2->DR;
	// printf("send music data\n"); //test
	for (; i < len;)
	{
		// SPI_I2S_SendData(SPI2, p_data[i++]);
		SPI2->DR = p_data[i++];

		// printf("0x%04x    ",p_data[i++]);	//test
/****** dummy read for clear the buffer ******/
		{
			SPITimeout = SPIT_FLAG_TIMEOUT;
			while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_RXNE) == RESET)
			{
				if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
			}
			temp = SPI2->DR;
		}
/****** end ******/
	}
	// printf("\n"); //test


	VS1053_xDCS(1);
	return 0;
}


uint8_t vs1053b_dat_write(uint8_t *buf,uint16_t len)
{
	uint16_t i = 0;
	uint8_t j = 0;
	uint16_t temp;


	for (; i++ < len;)
	{

		while(VS1053_DREQ ==0)
		{;}

		SPITimeout = SPIT_FLAG_TIMEOUT;
		while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_BSY) == SET)
		{
			if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(0);
		}

		VS1053_xDCS(0);

		SPI_I2S_SendData(SPI2, buf[i]);


/****** dummy read for clear the buffer ******/
		{
			SPITimeout = SPIT_FLAG_TIMEOUT;
			while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_RXNE) == RESET)
			{
				if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
			}
			temp = SPI_I2S_ReceiveData(SPI2);
		}
/****** end ******/

		VS1053_xDCS(1);
	}
	return 0;
}

uint8_t end_fill_byte_send(uint8_t send_byte,uint16_t cycle)	//32 byte at a time
{
	uint16_t i = 0;
	uint8_t j = 0;
	uint16_t temp;

	send_byte = send_byte;
	for (; i++ < cycle;)
	{
		while(VS1053_DREQ ==0)
		{;}

		SPITimeout = SPIT_FLAG_TIMEOUT;
		while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_BSY) == SET)
		{
			if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(0);
		}

		VS1053_xDCS(0);

		for (; j++ < 32;)
		{
			SPI_I2S_SendData(SPI2, send_byte);
		}
		j=0;

/****** dummy read for clear the buffer ******/
		{
			SPITimeout = SPIT_FLAG_TIMEOUT;
			while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_RXNE) == RESET)
			{
				if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
			}
			temp = SPI_I2S_ReceiveData(SPI2);
		}
/****** end ******/

		VS1053_xDCS(1);
	}
	return 0;
}

void vs1053b_stop(void)
{
	uint16_t temp = 0;
	uint16_t i = 0;
	uint8_t buf_init[32];
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

		for (i = 0; i < 65; i++)
		{
			vs1053_write_cmd(SPI_WRAMADDR, 0x1e06);
			temp = vs1053_read_reg(SPI_WRAM);
			memset(buf_init, temp & 0xFF, sizeof(uint8_t) * 32);           /* init */
			vs1053b_dat_write(buf_init, 32);
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
}

uint16_t vs1053_read_reg(uint8_t addr)
{
	uint16_t temp;
	uint8_t retry = 0;
	uint8_t buf[2] = {0};
	SPITimeout = SPIT_FLAG_TIMEOUT;
	
	temp = ( 0x0300 | ((uint16_t)addr) );
	printf("[Reading Reg1/2]Addr: 0x%04x\n",addr);

	buf[0] = (temp >> 8) & 0xFF;                                      /* get msb */
	buf[1] = (temp >> 0) & 0xFF;                                      /* get lsb */

	while(VS1053_DREQ == 0 && retry < 10)
	{
		vTaskDelay(10);
		retry++;
	}
	if (retry == 10)
	{
		while(1)
		{
			printf("read reg fail!\nDREQ error!\n");
			vTaskDelay(1000);
		}
	}

	while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_BSY) == SET)
	{
		if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(0);
	}
	VS1053_xDCS(1);
	VS1053_xCS(0);
	SPI_I2S_SendData(SPI2,buf[0]);
/****** dummy read for clear the buffer ******/
	{
		SPITimeout = SPIT_FLAG_TIMEOUT;
		while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_RXNE) == RESET)
		{
			if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
		}
		temp = SPI_I2S_ReceiveData(SPI2);
	}
/****** end ******/

	SPI_I2S_SendData(SPI2,buf[1]);
/****** dummy read for clear the buffer ******/
	{
		SPITimeout = SPIT_FLAG_TIMEOUT;
		while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_RXNE) == RESET)
		{
			if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
		}
		temp = SPI_I2S_ReceiveData(SPI2);
	}
/****** end ******/

	SPI_I2S_SendData(SPI2,Dummy_Byte);
	SPITimeout = SPIT_FLAG_TIMEOUT;
	/* 等待接收缓冲区非空，RXNE事件 */
	while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_RXNE) == RESET)
	{
		if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
	}
	buf[0] = SPI_I2S_ReceiveData(SPI2);

	SPI_I2S_SendData(SPI2,Dummy_Byte);
	SPITimeout = SPIT_FLAG_TIMEOUT;
	/* 等待接收缓冲区非空，RXNE事件 */
	while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_RXNE) == RESET)
	{
		if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
	}
	buf[1] = SPI_I2S_ReceiveData(SPI2);

	temp = (uint16_t) (buf[0] << 8) | buf[1];

	VS1053_xCS(1);
	printf("[Reading Reg2/2]received: 0x%04x\n",temp);
	vTaskDelay(3);
	return temp;
}

uint8_t vs1053_write_cmd(uint8_t addr,uint16_t data)
{
	uint16_t temp;
	uint8_t retry = 0;
	uint8_t buf[2] = {0};
	SPITimeout = SPIT_FLAG_TIMEOUT;

	temp = ( 0x0200 | (uint16_t)addr);
	printf("[Writing Reg]Addr: 0x%04x, Data: 0x%04x\n",addr,data);
	buf[0] = (temp >> 8) & 0xFF;                                      /* get msb */
	buf[1] = (temp >> 0) & 0xFF;                                      /* get lsb */

	while(VS1053_DREQ == 0 && retry < 10)
	{
		vTaskDelay(10);
		retry++;
	}
	if (retry == 10)
	{
		printf("write cmd failed\nDREQ error!\n");
		return 0xff;
	}

	VS1053_xDCS(1);
	VS1053_xCS(0);

	while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_BSY) == SET)
	{
		if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(0);
	}
	SPI_I2S_SendData(SPI2,buf[0]);

/****** dummy read for clear the buffer ******/
	{
		SPITimeout = SPIT_FLAG_TIMEOUT;
		while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_RXNE) == RESET)
		{
			if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
		}
		temp = SPI_I2S_ReceiveData(SPI2);
	}
/****** end ******/
	SPI_I2S_SendData(SPI2,buf[1]);

/****** dummy read for clear the buffer ******/
	{
		SPITimeout = SPIT_FLAG_TIMEOUT;
		while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_RXNE) == RESET)
		{
			if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
		}
		temp = SPI_I2S_ReceiveData(SPI2);
	}
/****** end ******/

	buf[0] = (data >> 8) & 0xFF;                                      /* get msb */
	buf[1] = (data >> 0) & 0xFF;                                      /* get lsb */
	SPITimeout = SPIT_FLAG_TIMEOUT;
	// while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_TXE) == RESET)
	while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_BSY) == SET)
	{
		if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(0);
	}
	SPI_I2S_SendData(SPI2,buf[0]);

/****** dummy read for clear the buffer ******/
	{
		SPITimeout = SPIT_FLAG_TIMEOUT;
		while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_RXNE) == RESET)
		{
			if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
		}
		temp = SPI_I2S_ReceiveData(SPI2);
	}
/****** end ******/
	SPI_I2S_SendData(SPI2,buf[1]);

/****** dummy read for clear the buffer ******/
	{
		SPITimeout = SPIT_FLAG_TIMEOUT;
		while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_RXNE) == RESET)
		{
			if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
		}
		temp = SPI_I2S_ReceiveData(SPI2);
	}
/****** end ******/



	VS1053_xCS(1);
	vTaskDelay(3);
	return 0;
}

uint8_t vs1053_read_write_byte(uint8_t tx_data)
{
	uint8_t rx_data = 0;
	uint8_t i = 0;
	for (i = 0; i < 8;i++)
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_13);	//clk
		if(tx_data&0x80)
			GPIO_SetBits(GPIOB,GPIO_Pin_15);	//MOSI
		else
			GPIO_ResetBits(GPIOB,GPIO_Pin_15);	//MOSI
		tx_data <<= 1;
		vTaskDelay(1);
		GPIO_SetBits(GPIOB,GPIO_Pin_13);	//clk
		vTaskDelay(1);
		rx_data <<= 1;
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14))	//MISO
			rx_data |= 0x01;
	}
	 GPIO_ResetBits(GPIOB,GPIO_Pin_13);	//clk
	return rx_data;
}


/**
 * @brief      convert the clock frequency to the register raw data
 * @param[in]  *handle pointer to a vs1053b handle structure
 * @param[in]  hz set hz
 * @param[out] *reg pointer to a register raw buffer
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 *             - 3 handle is not initialized
 * @note       none
 */
void vs1053b_clock_frequency_convert_to_register(float hz, uint16_t *reg)
{
	*reg = (uint16_t)((hz - 8000000.0f) / 4000.0f);      /* convert real data to register data */

}

/**
 * @brief     set clock frequency
 * @param[in] *handle pointer to a vs1053b handle structure
 * @param[in] clk clock frequency
 * @return    status code
 *            - 0 success
 *            - 1 set clock frequency failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 *            - 4 clk > 0x3FF
 * @note      none
 */
void vs1053b_set_clock_frequency(uint16_t clk)
{
	uint16_t temp;
	uint8_t buf[2];
	uint16_t prev;

	if (clk > 0x3FF)                                                  /* check clock */
	{
		printf("vs1053b: clk > 0x3FF.\n");               /* clk > 0x3FF */
		while(1)
			;
	}

	temp = vs1053_read_reg(SPI_CLOCKF);     /* read the clock */

	temp &= ~(0x3FF << 0);                                            /* clear the settings */
	temp |= clk << 0;                                                 /* set the config */

	vs1053_write_cmd(SPI_CLOCKF, temp);    /* write the clock */
}
