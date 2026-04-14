#ifndef _VS1053_H
#define _VS1053_H

#include <string.h>

#define SPI2_READ_BUFF_SIZE			14622

//VS10XX SCI register
#define SPI_MODE        	0x00
#define SPI_STATUS      	0x01
#define SPI_BASS        	0x02
#define SPI_CLOCKF      	0x03
#define SPI_DECODE_TIME 	0x04
#define SPI_AUDATA      	0x05	//单声道 Mono, 立体声 Stereo
#define SPI_WRAM        	0x06
#define SPI_WRAMADDR    	0x07
#define SPI_HDAT0       	0x08
#define SPI_HDAT1       	0x09

#define SPI_AIADDR      	0x0a
#define SPI_VOL         	0x0b
#define SPI_AICTRL0     	0x0c
#define SPI_AICTRL1     	0x0d
#define SPI_AICTRL2     	0x0e
#define SPI_AICTRL3     	0x0f

#define END_FULL_BYTE     	0x1e06

#define Dummy_Byte			0x0


  #define SPI2_SLAVE_DR_Base            0x4000380C 
  
/**
 * @brief vs1053b clock multiplier enumeration definition
 */
typedef enum
{
    VS1053B_CLOCK_MULTIPLIER_1P0_XTALI = 0x00,        /**< 1.0 xtali */
    VS1053B_CLOCK_MULTIPLIER_2P0_XTALI = 0x01,        /**< 2.0 xtali */
    VS1053B_CLOCK_MULTIPLIER_2P5_XTALI = 0x02,        /**< 2.5 xtali */
    VS1053B_CLOCK_MULTIPLIER_3P0_XTALI = 0x03,        /**< 3.0 xtali */
    VS1053B_CLOCK_MULTIPLIER_3P5_XTALI = 0x04,        /**< 3.5 xtali */
    VS1053B_CLOCK_MULTIPLIER_4P0_XTALI = 0x05,        /**< 4.0 xtali */
    VS1053B_CLOCK_MULTIPLIER_4P5_XTALI = 0x06,        /**< 4.5 xtali */
    VS1053B_CLOCK_MULTIPLIER_5P0_XTALI = 0x07,        /**< 5.0 xtali */
} vs1053b_clock_multiplier_t;

/**
 * @brief vs1053b allowed multiplier addition enumeration definition
 */
typedef enum
{
    VS1053B_ALLOWED_MULTIPLIER_ADDITION_NONE      = 0x00,        /**< no modification is allowed */
    VS1053B_ALLOWED_MULTIPLIER_ADDITION_1P0_XTALI = 0x01,        /**< 1.0 xtali */
    VS1053B_ALLOWED_MULTIPLIER_ADDITION_1P5_XTALI = 0x02,        /**< 1.5 xtali */
    VS1053B_ALLOWED_MULTIPLIER_ADDITION_2P0_XTALI = 0x03,        /**< 2.0 xtali */
} vs1053b_allowed_multiplier_addition_t;

#define VS1053B_BASIC_DEFAULT_PLAY_CLOCK_MULTIPLIER                            VS1053B_CLOCK_MULTIPLIER_3P5_XTALI                   /**< 3.5 xtali */
#define VS1053B_BASIC_DEFAULT_PLAY_ALLOWED_MULTIPLIER_ADDITION                 VS1053B_ALLOWED_MULTIPLIER_ADDITION_2P0_XTALI        /**< 2.0 xtali */
#define VS1053B_BASIC_DEFAULT_PLAY_CLOCK_FREQUENCY                             8000000.0f                                           /**< 8000000Hz */


#define MUSIC_DATA_LEN						(32)

/*等待超时时间*/
#define SPIT_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define SPIT_LONG_TIMEOUT         ((uint32_t)(10 * SPIT_FLAG_TIMEOUT))


//spi VS1053xCS use pin PB12
#define VS1053_xCS(x)								( (x)==1? (GPIO_SetBits(GPIOB,GPIO_Pin_12)) : (GPIO_ResetBits(GPIOB,GPIO_Pin_12)) )
//spi VS1053 DataCS use pin PD11
#define VS1053_xDCS(x)								( (x)==1? (GPIO_SetBits(GPIOD,GPIO_Pin_11)) : (GPIO_ResetBits(GPIOD,GPIO_Pin_11)) )
//spi VS1053 xRESET use pin PD13
#define VS1053_xRESET(x)							( (x)==1? (GPIO_SetBits(GPIOD,GPIO_Pin_13)) : (GPIO_ResetBits(GPIOD,GPIO_Pin_13)) )
//spi VS1053 DREQ use pin PD12
#define VS1053_DREQ									((GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12)))


extern uint8_t vs1053_read_write_byte(uint8_t tx_data);
extern uint16_t music_data[MUSIC_DATA_LEN];
extern uint16_t vs1053_read_reg(uint8_t addr);
extern uint8_t vs1053_write_cmd(uint8_t addr,uint16_t data);
extern uint8_t vs1053_send_music_data(uint8_t *p_data,uint16_t len);
extern uint8_t end_fill_byte_send(uint8_t send_byte, uint16_t cycle);
extern uint8_t vs1053b_dat_write(uint8_t *buf, uint16_t len);
void vs1053_soft_reset(void);
uint8_t vs1053_hardware_reset(void);
void vs1053_init(void);
extern void spi2_init(void);
void vs1053b_clock_frequency_convert_to_register(float hz, uint16_t *reg);
void vs1053b_set_clock_frequency(uint16_t clk);
extern void vs1053b_stop(void);
#endif /* _VS1053_H */
