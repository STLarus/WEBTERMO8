#ifndef DEFINE_H
#define DEFINE_H
#include "stm32f4xx.h"

#define ISKLJUCI		0	//statusi izlaznih releja
#define UKLJUCI			1

#define	GRIJANJE		1
#define	HLADJENJE		2

#define OPEN	1
#define CLOSE	2

#define	UKLJUCEN	1
#define	ISKLJUCEN 	2

//definicija alarmnih signala
#define TEMPHIGH    1
#define TEMPLOW     2
#define TEMPNORMAL  3
#define DOOROPEN    4
#define DOORCLOSE   5

#define	NEMA_ALARMA	1
#define	OTVOREN		2
#define	ZATVOREN	3
#define	ALARM		5

#define	NO_ALARM	0
#define	ALARM_HI	1
#define	ALARM_LOW 	2
#define 	ALARM_ERR	3

#define	SCAN_IO_TIME	10		//vrijeme skeniranja izlaza u ms
#define INT	1
#define	EXT	2

#define	LOG_TEMP	1
#define	LOG_RESET	2

#define	SVA_BUF_SIZE	3000

//#define IN1		GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9)
//#define IN2		GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10)
//#define IN3     GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11)

#define IN1_STATE   GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9)
#define IN2_STATE   GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10)
#define IN3_STATE   GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11)

//parametri mre�e
#define	EE_INIT			0x00
#define	EE_IP			0x04
#define EE_SUBNET		0x08
#define EE_GATEWAY		0x0C
#define	EE_DNS			0x10
#define	EE_HTTP_PORT	0x14
#define	EE_DHCP_STATE	0x16
#define	EE_DDNS_STATE	0x17
#define EE_ALARM_IP     0x18
#define EE_ALARM_PORT   0x1C
#define	EE_USERNAME		0x20
#define	EE_PASS 		0x30
#define	EE_DDNS_HOST	0x40
#define EE_OBJECT_NAME	0x60
#define EE_STATUS_TIME  0x87
#define EE_SCAN_TIME    0x89
#define EE_STATUS_SEND  0x91
#define EE_BACKUP_IP    0x92
#define EE_BACKUP_PORT  0x96
#define EE_BACKUP_TIME  0x98
#define EE_BACKUP_STAT  0x9A

//parametri senzora
#define EE_SENSOR_CODE  0x00
#define EE_LOW_ALARM    0x08
#define EE_HIGH_ALARM   0x09
#define EE_HIST         0x0B
#define EE_SENSOR_ACTIVE    0x0C
#define EE_CHANNEL      0x0D
#define EE_WAPP1_ON     0x10
#define EE_WAPP2_ON     0x11
#define EE_WAPP3_ON     0x12
#define EE_WAPP4_ON     0x13
#define EE_WAPP5_ON     0x14
#define EE_SMS1         0x20
#define EE_SMS2         0x21
#define EE_SMS3         0x22
#define EE_SMS4         0x23
#define EE_SMS5         0x24
#define EE_MEASURE_TYPE 0x25
#define EE_DELAY 	0x26
#define EE_CALIB        0x30
#define EE_SENSOR_NAME  0x40

//parametri SNTP
#define EE_SNTP_START	  0xE00
#define	EE_DST_STATE    EE_SNTP_START + 0x00
#define EE_TIME_ZONE    EE_SNTP_START + 0x01
#define EE_SNTP_STATE   EE_SNTP_START + 0x02
#define EE_DST_ACTIVETIME	EE_SNTP_START + 0x03
#define	EE_SNTP1        EE_SNTP_START + 0x20
#define EE_SNTP2        EE_SNTP_START + 0x40

//parametri SMS
#define 	EE_GSM_START	0xC00
#define 	EE_GSM_IP       EE_GSM_START + 0x00
#define	    EE_GSM_PORT     EE_GSM_START + 0x004
#define	    EE_GSM_UNAME    EE_GSM_START + 0x040
#define	    EE_GSM_PASS     EE_GSM_START + 0x050
#define	    EE_GSM1_NUM     EE_GSM_START + 0x060
#define	    EE_GSM2_NUM     EE_GSM_START + 0x080
#define	    EE_GSM3_NUM     EE_GSM_START + 0x0A0
#define	    EE_GSM4_NUM     EE_GSM_START + 0x0C0
#define	    EE_GSM5_NUM     EE_GSM_START + 0x0E0
#define	    EE_GSM1_NAME    EE_GSM_START + 0x100
#define	    EE_GSM2_NAME    EE_GSM_START + 0x120
#define	    EE_GSM3_NAME    EE_GSM_START + 0x140
#define	    EE_GSM4_NAME    EE_GSM_START + 0x160
#define	    EE_GSM5_NAME    EE_GSM_START + 0x180

//parametri WAPP
#define		EE_WAPP_START	0x1100
#define	    EE_WAPP1_NUM     EE_WAPP_START + 0x000
#define	    EE_WAPP1_KEY     EE_WAPP_START + 0x020
#define	    EE_WAPP1_NAME    EE_WAPP_START + 0x040
#define	    EE_WAPP2_NUM     EE_WAPP_START + 0x080
#define	    EE_WAPP2_KEY     EE_WAPP_START + 0x0A0
#define	    EE_WAPP2_NAME    EE_WAPP_START + 0x0C0
#define	    EE_WAPP3_NUM     EE_WAPP_START + 0x100
#define	    EE_WAPP3_KEY     EE_WAPP_START + 0x120
#define	    EE_WAPP3_NAME    EE_WAPP_START + 0x140
#define	    EE_WAPP4_NUM     EE_WAPP_START + 0x180
#define	    EE_WAPP4_KEY     EE_WAPP_START + 0x1A0
#define	    EE_WAPP4_NAME    EE_WAPP_START + 0x1C0
#define	    EE_WAPP5_NUM     EE_WAPP_START + 0x200
#define	    EE_WAPP5_KEY     EE_WAPP_START + 0x220
#define	    EE_WAPP5_NAME    EE_WAPP_START + 0x240

#define	    EE_GSM2_NUM     EE_GSM_START + 0x080
#define	    EE_GSM3_NUM     EE_GSM_START + 0x0A0
#define	    EE_GSM4_NUM     EE_GSM_START + 0x0C0
#define	    EE_GSM5_NUM     EE_GSM_START + 0x0E0
#define	    EE_GSM1_NAME    EE_GSM_START + 0x100
#define	    EE_GSM2_NAME    EE_GSM_START + 0x120
#define	    EE_GSM3_NAME    EE_GSM_START + 0x140
#define	    EE_GSM4_NAME    EE_GSM_START + 0x160
#define	    EE_GSM5_NAME    EE_GSM_START + 0x180

//alarmni ulazi
#define	EE_INPUT_START	0x1000
#define	EE_INPUT_NAME1	EE_INPUT_START + 0x00
#define	EE_INPUT_NAME2	EE_INPUT_START + 0x20
#define	EE_INPUT_NAME3	EE_INPUT_START + 0x40
#define EE_INPUT_ALARM1	EE_INPUT_START + 0x60
#define EE_INPUT_ALARM2	EE_INPUT_START + 0x61
#define EE_INPUT_ALARM3	EE_INPUT_START + 0x62
#define EE_INPUT_TIP1	EE_INPUT_START + 0x63
#define EE_INPUT_TIP2	EE_INPUT_START + 0x64
#define EE_INPUT_TIP3	EE_INPUT_START + 0x65
#define EE_INPUT_WAPP1	EE_INPUT_START + 0x66
#define EE_INPUT_WAPP2	EE_INPUT_START + 0x67
#define EE_INPUT_WAPP3	EE_INPUT_START + 0x68
#define EE_INPUT_WAPP4	EE_INPUT_START + 0x69
#define EE_INPUT_WAPP5	EE_INPUT_START + 0x6A
#define EE_INPUT_SMS1	EE_INPUT_START + 0x6B
#define EE_INPUT_SMS2	EE_INPUT_START + 0x6C
#define EE_INPUT_SMS3	EE_INPUT_START + 0x6D
#define EE_INPUT_SMS4	EE_INPUT_START + 0x6E
#define EE_INPUT_SMS5	EE_INPUT_START + 0x6F
#define EE_INPUT_DELAY1	EE_INPUT_START + 0x70
#define EE_INPUT_DELAY2	EE_INPUT_START + 0x72
#define EE_INPUT_DELAY3	EE_INPUT_START + 0x74


//MQTT
#define	EE_MQTT_START	0x1400
#define	EE_MQTT_SERVER	EE_MQTT_START + 0x00
#define	EE_MQTT_UNAME	EE_MQTT_START + 0x80
#define	EE_MQTT_PASS	EE_MQTT_START + 0xA0
#define	EE_MQTT_TOPIC	EE_MQTT_START + 0xC0
#define	EE_MQTT_ENABLE	EE_MQTT_START + 0x100
#define	EE_MQTT_PORT	EE_MQTT_START + 0x101
#define	EE_MQTT_QOS	EE_MQTT_START + 0x103
#define	EE_MQTT_KEEPALIVE	EE_MQTT_START + 0x104
#define	EE_MQTT_PERIOD	EE_MQTT_START + 0x105

// NVRAM
#define NV_LOGGER_INIT  0x04
#define NV_SUM_SEN1     0x08
#define NV_COUNT_SEN1   0x0C
#define NV_SUM_SEN2     0x10
#define NV_COUNT_SEN2   0x14
#define NV_SUM_SEN3     0x18
#define NV_COUNT_SEN3   0x1C
#define NV_SUM_SEN4     0x20
#define NV_COUNT_SEN4   0x24
#define NV_SUM_SEN5     0x28
#define NV_COUNT_SEN5   0x2C
#define NV_SUM_SEN6     0x30
#define NV_COUNT_SEN6   0x34
#define NV_SUM_SEN7     0x38
#define NV_COUNT_SEN7   0x3C
#define NV_SUM_SEN8     0x40
#define NV_COUNT_SEN8   0x44

#define NV_MKT_INIT         0x48
#define NV_MKT_COUNT_SEN1   0x4C
#define NV_MKT_COUNT_SEN2   0x50
#define NV_MKT_COUNT_SEN3   0x54
#define NV_MKT_COUNT_SEN4   0x58
#define NV_MKT_COUNT_SEN5   0x5c
#define NV_MKT_COUNT_SEN6   0x60
#define NV_MKT_COUNT_SEN7   0x64
#define NV_MKT_COUNT_SEN8   0x68
#define NV_MKT_SEN1         0x70
#define NV_MKT_SEN2         0xC0
#define NV_MKT_SEN3         0x110
#define NV_MKT_SEN4         0x1B0
#define NV_MKT_SEN5         0x200
#define NV_MKT_SEN6         0x250
#define NV_MKT_SEN7         0x2A0
#define NV_MKT_SEN8         0x340

#define	NV_DELAY_SEN1	0x400
#define	NV_DELAY_SEN2	0x404
#define	NV_DELAY_SEN3	0x408
#define	NV_DELAY_SEN4	0x40C
#define	NV_DELAY_SEN5	0x410
#define	NV_DELAY_SEN6	0x414
#define	NV_DELAY_SEN7	0x418
#define	NV_DELAY_SEN8	0x41C

#define	NV_DELAY_IN1	0x420
#define	NV_DELAY_IN2	0x424
#define	NV_DELAY_IN3	0x428

#define NVRAM_Write8(address, value)	(*(__IO uint8_t *) (BKPSRAM_BASE + (address)) = (value))
#define NVRAM_Read8(address)			(*(__IO uint8_t *) (BKPSRAM_BASE + address))
#define NVRAM_Write16(address, value)	(*(__IO uint16_t *) (BKPSRAM_BASE + (address)) = (value))
#define NVRAM_Read16(address)			(*(__IO uint16_t *) (BKPSRAM_BASE + address))
#define NVRAM_Write32(address, value)	(*(__IO uint32_t *) (BKPSRAM_BASE + (address)) = (value))
#define NVRAM_Read32(address)			(*(__IO uint32_t *) (BKPSRAM_BASE + address))
#define NVRAM_WriteFloat(address, value)	(*(__IO float *) (BKPSRAM_BASE + (address)) = (value))
#define NVRAM_ReadFloat(address)			(*(__IO float *) (BKPSRAM_BASE + address))
/**
 * @brief  Reads 32-bit float value from backup SRAM at desired location
 *
 * Parameters:
 * @param  address: Address where to save data in SRAM.
 * 		      - Value between 0 and TM_BKPSRAM_GetMemorySize() - 4 is valid, if more, HardFault error can happen.
 * @retval 32-bit float value at specific location
 * @note   Defined as macro for faster execution
 */
#define TM_BKPSRAM_ReadFloat(address)			(*(__IO float *) (BKPSRAM_BASE + address))

//definicije za LittleFS
#define	PARAM_OFFSET		0x2000000	//x1F00000	//	32MB - 1024 K  (1M) prostor u memoriji za parametre
#define LFS_BLOCKS_NUM	PARAM_OFFSET/4096
#define LFS_BLOCK_SIZE	4096

#define	DEFAULT_USER_NAME	"s"
#define	DEFAULT_PASSWORD	"s"
#define DEFAULT_NAZIV_OBJEKTA	"Larus Webtermo"
#define DEFAULT_DDNS_HOST	"myhostname.com"

#define VERZIJA		"VER-3.0 01082023"
#define DATUM_VERZIJE	20230801

#define DS1307_BACKUP_REGISTER  RTC_BKP_DR8
#define DS1307_EXIST    13

#define ATTR_TIMESTAMP 0x74

#define WAPP_BUSY	1
#define WAPP_FREE	0

#define IC_TIMEOUT_MS	50
#define I2C_TIMEOUT_ERR	23


#endif

