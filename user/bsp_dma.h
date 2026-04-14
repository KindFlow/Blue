#ifndef _BSP_DMA_H
#define _BSP_DMA_H

#define DEBUG_DMAx						DMA1_Channel5
#define DEBUG_DMA_IRQ					DMA1_Channel5_IRQn
#define DEBUG_DMA_IRQHandler			DMA1_Channel5_IRQHandler
#define DEBUG_DMA_FALG_TC				DMA1_FLAG_TC5

void uart1_DMA1_init(void);
void uart1_DMA1_renew(void);
void uart1_DMA1_init(void);


#endif /* "bsp_dma.h" _BSP_DMA_H */
