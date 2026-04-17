#ifndef _BSP_GPIO_H
#define _BSP_GPIO_H


extern void gpio_init(void);

#define KEY_PA0_PRESS			(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)?1:0)
#define KEY_PE3_PRESS			(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)?0:1)
#define KEY_PE4_PRESS			(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)?0:1)
#endif /* "bsp_gpio.h" _BSP_GPIO_H */
