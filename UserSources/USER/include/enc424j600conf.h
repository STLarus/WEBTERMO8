//*****************************************************************************
//
// File Name    : 'ENC424J600conf.h'
// Title                : Microchip ENC424J600 Ethernet Interface Driver Configuration
// Author               : Pascal Stang
//*****************************************************************************



#ifndef ENC424J600CONF_H
#define ENC424J600CONF_H

#include "encx24j600.h"

extern void enc424j600Init(void);
extern u16 enc424j600PacketReceive(u16 , u08* );
void enc424j600PacketSend(u16 , u08* );
extern u16 enc424j600IfPacketArrived(void); 

// ENC424J600 config
#define RAMSIZE                         (0x6000)
#define TXSTART                         (0x0000)
#define RXSTART                         (0x0600)        // Should be an even memory address

#define sENC_SPI                           SPI3
#define sENC_SPI_CLK                       RCC_APB1Periph_SPI3
#define sENC_SPI_CLK_INIT                  RCC_APB1PeriphClockCmd

#define sENC_SPI_SCK_PIN                   GPIO_Pin_10
#define sENC_SPI_SCK_GPIO_PORT             GPIOC
#define sENC_SPI_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOC
#define sENC_SPI_SCK_SOURCE                GPIO_PinSource10
#define sENC_SPI_SCK_AF                    GPIO_AF_SPI3

#define sENC_SPI_MISO_PIN                  GPIO_Pin_11
#define sENC_SPI_MISO_GPIO_PORT            GPIOC
#define sENC_SPI_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOC
#define sENC_SPI_MISO_SOURCE               GPIO_PinSource11
#define sENC_SPI_MISO_AF                   GPIO_AF_SPI3

#define sENC_SPI_MOSI_PIN                  GPIO_Pin_12
#define sENC_SPI_MOSI_GPIO_PORT            GPIOC
#define sENC_SPI_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOC
#define sENC_SPI_MOSI_SOURCE               GPIO_PinSource12
#define sENC_SPI_MOSI_AF                   GPIO_AF_SPI3

#define sENC_CS_PIN                        GPIO_Pin_9
#define sENC_CS_GPIO_PORT                  GPIOC
#define sENC_CS_GPIO_CLK                   RCC_AHB1Periph_GPIOC



#define ENC_CS_LOW()       GPIO_ResetBits(GPIOC,GPIO_Pin_9)
#define ENC_CS_HIGH()      GPIO_SetBits(GPIOC,GPIO_Pin_9)   


#define	SPDR		SPI3->DR
#define	SPSR		SPI3->SR


#endif

