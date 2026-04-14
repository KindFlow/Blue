#include "bsp_system_init.h"
#include "stm32f10x_rcc.h"

void cpu_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);

}

