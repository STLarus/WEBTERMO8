/************************************************************************
 *
 *    DRAJVER.C
 *
 *
 ************************************************************************/
#include "stm32f4xx.h"
#include "datatypes.h"
#include "pt.h"
#include "timer.h"
#include "strukture.h"
#include <string.h>
#include <stdio.h>
#include <stm32f4xx_i2c.h>

#define SLAVE_ADDRESS 0x50      //A0 je prava adresa, ovo je pomaknuto za 1 u desno
#define I2C_PORT    I2C1

#define	DS2482		0x30
#define I2CREAD		1
#define I2CWRITE	0
#define I2C_PORT    I2C1

//DS2482 komande
#define CMD_DRST	0xF0
#define CMD_WCFG	0xD2
#define CMD_1WRS	0xB4
#define CMD_1WRB	0x96
#define CMD_1WWB	0xA5
#define CMD_SRP		0xE1

// DS2482 status bits
#define STATUS_1WB  0x01
#define STATUS_PPD  0x02
#define STATUS_SD   0x04
#define STATUS_LL   0x08
#define STATUS_RST  0x10
#define STATUS_SBR  0x20
#define STATUS_TSB  0x40
#define STATUS_DIR  0x80

//#define I2C_WRITE 0
//#define I2C_READ  1

#define DOOR_OPEN   0
#define DOOR_CLOSE  1

#define	LED_ON		GPIO_ResetBits(GPIOE, GPIO_Pin_8)
#define LED_OFF		GPIO_SetBits(GPIOE, GPIO_Pin_8)

#define SDA_0()        GPIO_ResetBits(GPIOB,GPIO_Pin_7)
#define SDA_1()      GPIO_SetBits(GPIOB,GPIO_Pin_7)
#define SCL_0()        GPIO_ResetBits(GPIOB,GPIO_Pin_6)
#define SCL_1()      GPIO_SetBits(GPIOB,GPIO_Pin_6)

// **************** I2C1 DEFINICIJE
//#define		BIT6        0x0040
//#define  	BIT7        0x0080
//#define   SDA1	            BIT7
//#define   SCL1               BIT6
//#define   SDA1_1                GPIOB->ODR |= SDA1
//#define   SDA1_0                GPIOB->ODR &= (~SDA1) | 0xFFFFFF7F
//#define   SDA1_DATA             (GPIOB->IDR & SDA1)
//#define   SCL1_1                GPIOB->ODR |= SCL1
//#define   SCL1_0                GPIOB->ODR &= (~SCL1) | 0xFFFFFFBF

#define DELAY_2		delay_us(2);
#define	ONE_WIRE_DELAY	5		//Ã¨ekanje na odgovor DS2482, prije nego se funkcija prekine
#define	DELAY_COUNT_NUM	100*5	//broj prolaza kroz petlju Ã¨ekanja od 10uS

#define ACK		0
#define NACK	1

extern uint32_t I2CTimer;

UINT16 delay_count;
unsigned char I2CBuf[10];
UINT8 I2C_Err_flg;
extern void Delay(uint32_t);
extern void gisAlarm(uint8_t sid, uint8_t atype, float avalue);
struct timer LEDTimer, OneWireTimer, IN1Timer, IN2Timer, IN3Timer;
struct pt pt_ControlLED, pt_IN1, pt_IN2, pt_IN3;

/************************************************************************
 Function:       Bcd2Bin
 Parameters:     x.... BCD broj za konverziju
 Returns:        Binarni broj
 ************************************************************************/
UINT8 Bcd2Bin(UINT8 val) //konverzija BCD u binarni
    {
    return val - 6 * (val >> 4);
    } /***** Bcd2Bin() *****/

/************************************************************************
 Function:       Bin2Bcd
 Parameters:     x.... binarni broj za konverziju
 Returns:        Bcd broj
 ************************************************************************/
UINT8 Bin2Bcd(UINT8 val)
    {
    return val + 6 * (val / 10);
    }/***** Bin2Bcd() *****/

/************************************************************************
 Function:       delay_us
 Parameters:     value.... broj mikrosekundi za kašnjenje
 Returns:        none
 ************************************************************************/
void delay_us(uint16_t value)
    {
    value = value * 2;
    while (value--)
	{
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();

	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	}
    }/***** delay_us() *****/

void delay_ms(unsigned int n)
    {
    while (n--)
	{
	delay_us(1000);
	}
    }/***** delay_ms() *****/

unsigned char DS2482_channel_select(unsigned char chnl)
    {
    unsigned char adrs;
    if (chnl == 1)
	chnl = 4;
    else if (chnl == 2)
	chnl = 3;
    else if (chnl == 3)
	chnl = 2;
    else if (chnl == 4)
	chnl = 1;
    if (chnl == 1)
	adrs = DS2482;
    else if (chnl == 2)
	adrs = DS2482 + 4;
    else if (chnl == 3)
	adrs = DS2482 + 2;
    else if (chnl == 4)
	adrs = DS2482 + 6;
    return adrs;
    }/***** DS2482_channel_select() *****/

void I2Cinit(void)
    {
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    //GPIOB->BSRRL = BIT_6 | BIT_7;                            // SDA, SCL -> hi
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // SDA, SCL def
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;           // alternate function
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;           // use open drain !
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);  // PB6:I2C1_SCL
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);  // PB7:I2C1_SDA

    I2C_InitStructure.I2C_ClockSpeed = 400000;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitStructure.I2C_OwnAddress1 = 0;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Disable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitStructure);
    I2C_Cmd(I2C1, ENABLE);
    }

/**
 * Funkcija cita stanje SDA linije kao obicnog porta.
 */
char SDA_State(void)
    {
    if ((GPIOB->IDR & 0x0080))
	return 1;
    else
	return 0;
    }/***** SDA_State() *****/

/**
 * Resetira I2C chipove ako je koji zbog prekida rada ostao zaglavljen i nije spreman primit START impuls.
 */

void i2c_reset(void)
    {
    char i;

    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;
    I2C_DeInit(I2C1);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    SDA_1();  //Tristate data bus giving it a 1 level
    for (i = 0; i < 9; i++)
	{
	SCL_1();	//SCL = 1;  //Tristate clock line making it a 1
	delay_ms(5);
	SCL_0();	//SCL = 0;  //Force clock line low
	delay_ms(5);
	if (SDA_State())
	    break;     //Check SDA line if EEPROM has released it it goes high
	}
    //i2c_stop();     //Send stop condition
    /* I2C Stop condition, clock goes high when data is low */
    SCL_0();	//I2C_CLK = LOW;
    SDA_0();	//I2C_DATA = LOW;
    SCL_1();	//I2C_CLK = HIGH;
    SDA_1();	//I2C_DATA = HIGH;

    }

/************************************************************************
 Function:       OWreset
 Parameters:     channel..... kanal (1-4) sa kojeg se Ã¨ita
 Returns:        Resetira 1 wire bus
 ************************************************************************/
unsigned char OWreset(unsigned char channel)
    {
    unsigned char status, chipAdr;
    chipAdr = DS2482_channel_select(channel);
    I2CTimer = 1;
    while (I2C_GetFlagStatus(I2C_PORT, I2C_FLAG_BUSY))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_Send7bitAddress(I2C_PORT, chipAdr, I2C_Direction_Transmitter);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    //komanda
    I2C_SendData(I2C_PORT, CMD_1WRS);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_GenerateSTART(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_Send7bitAddress(I2C_PORT, chipAdr, I2C_Direction_Receiver);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_AcknowledgeConfig(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    status = I2C_ReceiveData(I2C_PORT);

    delay_count = 0;
    do
	{
	I2CTimer = 1;
	while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
	    {
	    if (I2CTimer > IC_TIMEOUT_MS)
		{
		I2C_GenerateSTOP(I2C_PORT, ENABLE);
		I2CTimer = 0;
		return I2C_TIMEOUT_ERR;
		}

	    };
	status = I2C_ReceiveData(I2C_PORT);
	delay_us(100);
	if (++delay_count > DELAY_COUNT_NUM)
	    {
	    I2C_error = 1;
	    //__enable_irq();
	    return status;
	    }
	}
    while (status & STATUS_1WB);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_AcknowledgeConfig(I2C_PORT, DISABLE);
    status = I2C_ReceiveData(I2C_PORT);
    I2C_AcknowledgeConfig(I2C_PORT, DISABLE);
    status = I2C_ReceiveData(I2C_PORT);
    //stop
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    return 0;
    }/***** OWreset() *****/

/************************************************************************
 Function:       OWWriteByte
 Parameters:     channel..... kanal (1-4) sa kojeg se Ã¨ita
 dta......podatak koji se upisuje
 Returns:        Status komunikacije
 ************************************************************************/
unsigned char OWWriteByte(unsigned char channel, unsigned char dta)
    {
    unsigned char status, chipAdr;
    chipAdr = DS2482_channel_select(channel);
    I2CTimer = 1;
    while (I2C_GetFlagStatus(I2C_PORT, I2C_FLAG_BUSY))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_Send7bitAddress(I2C_PORT, chipAdr, I2C_Direction_Transmitter);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    //komanda
    I2C_SendData(I2C_PORT, CMD_1WWB);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_SendData(I2C_PORT, dta);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_GenerateSTART(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_Send7bitAddress(I2C_PORT, chipAdr, I2C_Direction_Receiver);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_AcknowledgeConfig(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    status = I2C_ReceiveData(I2C_PORT);

    delay_count = 0;
    do
	{
	I2CTimer = 1;
	while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
	    {
	    if (I2CTimer > IC_TIMEOUT_MS)
		{
		I2C_GenerateSTOP(I2C_PORT, ENABLE);
		I2CTimer = 0;
		return I2C_TIMEOUT_ERR;
		}

	    };
	status = I2C_ReceiveData(I2C_PORT);
	delay_us(10);
	if (++delay_count > DELAY_COUNT_NUM)
	    {
	    I2C_error = 1;
	    //__enable_irq();
	    return status;
	    }
	}
    while (status & STATUS_1WB);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_AcknowledgeConfig(I2C_PORT, DISABLE);
    status = I2C_ReceiveData(I2C_PORT);

    //stop
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    return 0;
    }/***** OWWriteByte() *****/

/************************************************************************
 Function:       OWReadByte
 Parameters:     channel..... kanal (1-4) sa kojeg se Ã¨ita
 &dta......pointer na adresu na koju se upisuje proÃ¨itani podatak
 Returns:        Status komunikacije
 ************************************************************************/
unsigned char OWReadByte(unsigned char channel, unsigned char *dta)
    {
    unsigned char status, chipAdr;
    chipAdr = DS2482_channel_select(channel);
    I2CTimer = 1;
    while (I2C_GetFlagStatus(I2C_PORT, I2C_FLAG_BUSY))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_Send7bitAddress(I2C_PORT, chipAdr, I2C_Direction_Transmitter);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    //komanda
    I2C_SendData(I2C_PORT, CMD_1WRB);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_GenerateSTART(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_Send7bitAddress(I2C_PORT, chipAdr, I2C_Direction_Receiver);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_AcknowledgeConfig(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    status = I2C_ReceiveData(I2C_PORT);

    delay_count = 0;
    do
	{
	I2CTimer = 1;
	while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
	    {
	    if (I2CTimer > IC_TIMEOUT_MS)
		{
		I2C_GenerateSTOP(I2C_PORT, ENABLE);
		I2CTimer = 0;
		return I2C_TIMEOUT_ERR;
		}

	    };
	status = I2C_ReceiveData(I2C_PORT);
	delay_us(10);
	if (++delay_count > DELAY_COUNT_NUM)
	    {
	    I2C_error = 1;
	    //__enable_irq();
	    return status;
	    }
	}
    while (status & STATUS_1WB);

    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_AcknowledgeConfig(I2C_PORT, DISABLE);
    status = I2C_ReceiveData(I2C_PORT);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_AcknowledgeConfig(I2C_PORT, ENABLE);
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_Send7bitAddress(I2C_PORT, chipAdr, I2C_Direction_Transmitter);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    //komanda
    I2C_SendData(I2C_PORT, CMD_SRP);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_SendData(I2C_PORT, 0xE1);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_GenerateSTART(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_Send7bitAddress(I2C_PORT, chipAdr, I2C_Direction_Receiver);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_AcknowledgeConfig(I2C_PORT, DISABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    *dta = I2C_ReceiveData(I2C_PORT);

    //stop
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    return 0;
    }/***** OWReadByte() *****/

/************************************************************************
 Function:       DS2482_reset
 Parameters:     channel......adresa chipa koji se resetira
 Returns:        Status komunikacije
 ************************************************************************/
unsigned char DS2482_reset(unsigned char channel)
    {
    uint8_t chipAdr, status;
    chipAdr = DS2482_channel_select(channel);
    I2CTimer = 1;
    while (I2C_GetFlagStatus(I2C_PORT, I2C_FLAG_BUSY))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_GenerateSTART(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_Send7bitAddress(I2C_PORT, chipAdr, I2C_Direction_Transmitter);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    //komanda
    I2C_SendData(I2C_PORT, CMD_DRST);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    //ponovni start
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_Send7bitAddress(I2C_PORT, chipAdr, I2C_Direction_Receiver);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_AcknowledgeConfig(I2C_PORT, DISABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    status = I2C_ReceiveData(I2C_PORT);
    //stop
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    return ((status & 0xF7) == 0x10);
    }/***** DS2482_reset() *****/

/************************************************************************
 Function:       DS2482_init
 Parameters:     channel......adresa chipa koji se resetira
 Returns:        Status komunikacije
 ************************************************************************/
unsigned char DS2482_init(unsigned char channel)
    {
    uint8_t chipAdr, status;
    chipAdr = DS2482_channel_select(channel);
    I2CTimer = 1;
    while (I2C_GetFlagStatus(I2C_PORT, I2C_FLAG_BUSY))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_Send7bitAddress(I2C_PORT, chipAdr, I2C_Direction_Transmitter);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    //komanda
    I2C_SendData(I2C_PORT, CMD_WCFG);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_SendData(I2C_PORT, 0xA5);    //SPU+APU
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    //ponovni start
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    I2C_Send7bitAddress(I2C_PORT, chipAdr, I2C_Direction_Receiver);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};

    I2C_AcknowledgeConfig(I2C_PORT, DISABLE);
    I2CTimer = 1;
    while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
	if (I2CTimer > IC_TIMEOUT_MS)
	    {
	    I2C_GenerateSTOP(I2C_PORT, ENABLE);
	    I2CTimer = 0;
	    return I2C_TIMEOUT_ERR;
	    }

	};
    status = I2C_ReceiveData(I2C_PORT);
    //stop
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    return ((status & 0xF7) == 0x10);
    }/***** DS2482_init() *****/

void getTime(unsigned char *hour, unsigned char *min, unsigned char *sec)
    {
    RTC_TimeTypeDef RTC_Time;

    RTC_Time.RTC_H12 = RTC_HourFormat_24;

    RTC_GetTime(RTC_Format_BIN, &RTC_Time);

    *hour = RTC_Time.RTC_Hours;
    *min = RTC_Time.RTC_Minutes;
    *sec = RTC_Time.RTC_Seconds;
    }/***** getTime() *****/

void getDate(unsigned char *year, unsigned char *month, unsigned char *date,
	unsigned char *day)
    {
    RTC_DateTypeDef RTC_Date;

    RTC_GetDate(RTC_Format_BIN, &RTC_Date);

    *year = RTC_Date.RTC_Year;
    *month = RTC_Date.RTC_Month;
    *date = RTC_Date.RTC_Date;
    *day = RTC_Date.RTC_WeekDay;
    }/***** getDate() *****/

/************************************************************************
 Function:       getDateASCII
 Parameters:     *dtbuf......pointer na adresu na koju upisuje ASCII datum
 format upisa je DD.MM.YYYY
 Returns:        Status komunikacije
 ************************************************************************/
void getDateASCII(char *dtbuf)
    {
    char tbf[8];
    getDate(&godina, &mjesec, &datum, &dan);
    sprintf((char*) dtbuf, "%d", datum);
    strcat(dtbuf, ".");
    sprintf((char*) tbf, "%d", mjesec);
    strcat(dtbuf, tbf);
    strcat(dtbuf, ".");
    sprintf((char*) tbf, "%d", godina + 2000);
    strcat(dtbuf, tbf);
    }    //***** getDateASCII() *****/

/************************************************************************
 Function:       getTimeASCII
 Parameters:     *dtbuf......pointer na adresu na koju upisuje ASCII vrijeme
 format upisa je HH:MM:SS
 Returns:        Status komunikacije
 ************************************************************************/
void getTimeASCII(char *dtbuf)
    {
    char tbf[8];
    getTime(&sat, &minuta, &sekunda);
    sprintf((char*) dtbuf, "%d", sat);
    strcat(dtbuf, ":");
    sprintf((char*) tbf, "%d", minuta);
    strcat(dtbuf, tbf);
    strcat(dtbuf, ":");
    sprintf((char*) tbf, "%d", sekunda);
    strcat(dtbuf, tbf);
    }    //***** getTimeASCII() *****/

void setDate(unsigned char year, unsigned char month, unsigned char date,
	unsigned char day)
    {
    RTC_DateTypeDef RTC_Date;

    RTC_Date.RTC_Year = year;
    RTC_Date.RTC_Month = month;
    RTC_Date.RTC_Date = date;
    RTC_Date.RTC_WeekDay = day;
    RTC_SetDate(RTC_Format_BIN, &RTC_Date);
    }/* setDate() */

void setTime(unsigned char hour, unsigned char min, unsigned char sec)
    {
    RTC_TimeTypeDef RTC_Time;

    RTC_Time.RTC_H12 = RTC_HourFormat_24;
    RTC_Time.RTC_Hours = hour;
    RTC_Time.RTC_Minutes = min;
    RTC_Time.RTC_Seconds = sec;
    RTC_SetTime(RTC_Format_BIN, &RTC_Time);

    }/***** setTime() *****/

/********************************************************************************
 Function:       ptIN1
 Purpose:        OÄ�itava stanje ulazna.
 *********************************************************************************/
PT_THREAD(ptIN1xxx(struct pt *pt))
    {
    PT_BEGIN(pt)
    ;
    PT_YIELD(pt);
    PT_WAIT_UNTIL(pt, IN1_STATE!=0);
    timer_set(&IN1Timer, 50);
    PT_WAIT_UNTIL(pt, timer_expired(&IN1Timer));
    if (IN1_STATE > 0)
	{
	input1 = DOOR_CLOSE;
//    gisAlarm(1, DOORCLOSE, 1);
	}

    else
	PT_RESTART(pt);
    PT_WAIT_UNTIL(pt, (IN1_STATE==0));
    timer_set(&IN1Timer, 50);
    PT_WAIT_UNTIL(pt, timer_expired(&IN1Timer));
    if (IN1_STATE == 0)
	{
	input1 = DOOR_OPEN;
//    gisAlarm(1, DOOROPEN, 0);
	}

PT_END(pt);
}/***** ptIN1() *****/

/********************************************************************************
 Function:       ptIN2
 Purpose:        OÄ�itava stanje ulazna.
 *********************************************************************************/
PT_THREAD(ptIN2xxxx(struct pt *pt))
{
PT_BEGIN(pt)
;
PT_YIELD(pt);
PT_WAIT_UNTIL(pt, IN2_STATE!=0);
timer_set(&IN2Timer, 50);
PT_WAIT_UNTIL(pt, timer_expired(&IN2Timer));
if (IN2_STATE > 0)
    {
    input2 = DOOR_CLOSE;
//gisAlarm(2, DOORCLOSE, 1);
    }

else
    PT_RESTART(pt);
PT_WAIT_UNTIL(pt, (IN2_STATE==0));
timer_set(&IN2Timer, 50);
PT_WAIT_UNTIL(pt, timer_expired(&IN2Timer));
if (IN2_STATE == 0)
    {
    input2 = DOOR_OPEN;
//gisAlarm(2, DOOROPEN, 0);
    }

PT_END(pt);
}/***** ptIN2() *****/
/********************************************************************************
 Function:       ptIN3
 Purpose:        OÄ�itava stanje ulazna.
 *********************************************************************************/
PT_THREAD(ptIN3xxx(struct pt *pt))
{
PT_BEGIN(pt)
;
PT_YIELD(pt);
PT_WAIT_UNTIL(pt, IN3_STATE!=0);
timer_set(&IN3Timer, 50);
PT_WAIT_UNTIL(pt, timer_expired(&IN3Timer));
if (IN3_STATE > 0)
{
input3 = DOOR_CLOSE;
//gisAlarm(3, DOORCLOSE, 1);
}

else
PT_RESTART(pt);
PT_WAIT_UNTIL(pt, (IN3_STATE==0));
timer_set(&IN3Timer, 50);
PT_WAIT_UNTIL(pt, timer_expired(&IN3Timer));
if (IN3_STATE == 0)
{
input3 = DOOR_OPEN;
//gisAlarm(3, DOOROPEN, 0);
}

PT_END(pt);
}/***** ptIN3() *****/

/********************************************************************************
 Function:       ptIN1
 Purpose:        OÄ�itava stanje ulazna.
 *********************************************************************************/
PT_THREAD(ptIN1(struct pt *pt))
{
PT_BEGIN(pt)
;
PT_YIELD(pt);
PT_WAIT_UNTIL(pt, in1State != in1OldState);
timer_set(&IN1Timer, 50);
PT_WAIT_UNTIL(pt, timer_expired(&IN1Timer));
if (IN1_STATE == in1State)
{
if (in1State == 0)
{
input1 = DOOR_OPEN;
//gisAlarm(1, DOOROPEN, 0);
}
else
{
input1 = DOOR_CLOSE;
//gisAlarm(1, DOORCLOSE, 1);
}

}
else
PT_RESTART(pt);
PT_END(pt);
}/***** ptIN1() *****/

/********************************************************************************
 Function:       ptIN2
 Purpose:        OÄ�itava stanje ulazna.
 *********************************************************************************/
PT_THREAD(ptIN2(struct pt *pt))
{
PT_BEGIN(pt)
;
PT_YIELD(pt);
PT_WAIT_UNTIL(pt, in2State != in2OldState);
timer_set(&IN2Timer, 50);
PT_WAIT_UNTIL(pt, timer_expired(&IN2Timer));
if (IN2_STATE == in2State)
{
if (in2State == 0)
{
input2 = DOOR_OPEN;
//gisAlarm(2, DOOROPEN, 0);
}
else
{
input2 = DOOR_CLOSE;
//gisAlarm(2, DOORCLOSE, 1);
}

}
else
PT_RESTART(pt);
PT_END(pt);
}/***** ptIN2() *****/

/********************************************************************************
 Function:       ptIN3
 Purpose:        OÄ�itava stanje ulazna.
 *********************************************************************************/
PT_THREAD(ptIN3(struct pt *pt))
{
PT_BEGIN(pt)
;
PT_YIELD(pt);
PT_WAIT_UNTIL(pt, in3State != in3OldState);
timer_set(&IN3Timer, 50);
PT_WAIT_UNTIL(pt, timer_expired(&IN3Timer));
if (IN3_STATE == in3State)
{
if (in3State == 0)
{
input3 = DOOR_OPEN;
//gisAlarm(3, DOOROPEN, 0);
}
else
{
input3 = DOOR_CLOSE;
//gisAlarm(3, DOORCLOSE, 1);
}

}
else
PT_RESTART(pt);
PT_END(pt);
}/***** ptIN3() *****/

void input_poll(void)
{
in1State = IN1_STATE;
in2State = IN2_STATE;
in3State = IN3_STATE;
ptIN1(&pt_IN1);
ptIN2(&pt_IN2);
ptIN3(&pt_IN3);
in1OldState = in1State;
in2OldState = in2State;
in3OldState = in3State;
}
