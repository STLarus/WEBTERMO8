#include "timer.h"
#include "pt.h"
#include "enc424j600conf.h"
#include "net.h"
#include "stm32f4xx_conf.h"
#include "strukture.h"
#include "boot.h"
#include "smtp.h"
#include "sntp.h"
#include "http_client.h"
#include "logger.h"
#include "backup.h"
#include <stdint.h>
#include <string.h>
#include "lfs.h"
#include <stdio.h>
#include <math.h>
#include <W25Q256.h>

#pragma section = ".bss"

#define LED_ON()       GPIO_ResetBits(GPIOB,GPIO_Pin_9)
#define LED_OFF()      GPIO_SetBits(GPIOB,GPIO_Pin_9)

RTC_InitTypeDef RTC_InitStructure;
extern uint32_t _sbss, _ebss;
extern uint32_t _sccmram, _eccmram;

extern struct netif netif;
extern struct MAC_ADDR
    {
	u08 v[6];
    } mac_addr;

extern UINT16 ReadTemperature();
extern void MqttPublish(uint8_t);
unsigned char hr, mn, sc, oldmin; //varijable vezane uz refresh komunikacije
unsigned char IP[4];
unsigned char MAC[6];
unsigned int i, sms_old_min, old_reset_min, old_haccp_min;
unsigned char oldscanhr, oldhr, sntphr, sntpoldhr;
struct pt pt_ToggleLed, pt_ArpGratuitous, pt_TestLink;
struct timer ledTimer, arpTimer, linkTimer; //, mqttTimer;

PT_THREAD( ptArpGratuitous(struct pt*));
PT_THREAD( ptTestLink(struct pt*));
PT_THREAD( ptToggleLed(struct pt*));
PT_THREAD( ptMQTT(struct pt*));

/**
 * @brief  Configures TIM5 to measure the LSI oscillator frequency.
 * @param  None
 * @retval LSI Frequency
 */
//=========================================================================== Independent Watchdog Configuration
// <e0> Independent Watchdog Configuration
//   <o1> IWDG period [us] <125-32000000:125>
//   <i> Set the timer period for Independent Watchdog.
//   <i> Default: 1000000  (1s)
// </e>
#define __IWDG_SETUP              1
#define __IWDG_PERIOD             0x001E8480
/*----------------------------------------------------------------------------
 Define  IWDG PR and RLR settings
 *----------------------------------------------------------------------------*/
#if   (__IWDG_PERIOD >  16384000UL)
#define __IWDG_PR             (6)
#define __IWDGCLOCK (32000UL/256)
#elif (__IWDG_PERIOD >   8192000UL)
#define __IWDG_PR             (5)
#define __IWDGCLOCK (32000UL/128)
#elif (__IWDG_PERIOD >   4096000UL)
#define __IWDG_PR             (4)
#define __IWDGCLOCK  (32000UL/64)
#elif (__IWDG_PERIOD >   2048000UL)
#define __IWDG_PR             (3)
#define __IWDGCLOCK  (32000UL/32)
#elif (__IWDG_PERIOD >   1024000UL)
#define __IWDG_PR             (2)
#define __IWDGCLOCK  (32000UL/16)
#elif (__IWDG_PERIOD >    512000UL)
#define __IWDG_PR             (1)
#define __IWDGCLOCK   (32000UL/8)
#else
#define __IWDG_PR             (0)
#define __IWDGCLOCK   (32000UL/4)
#endif
#define __IWGDCLK  (32000UL/(0x04<<__IWDG_PR))
#define __IWDG_RLR (__IWDG_PERIOD*__IWGDCLK/1000000UL-1)
/*----------------------------------------------------------------------------
 STM32 Independent watchdog setup.
 initializes the IWDG register
 *----------------------------------------------------------------------------*/
static void IwdgSetup(void)
    {
    IWDG->KR = 0x5555;		// enable write to PR, RLR
    IWDG->PR = __IWDG_PR;	// Init prescaler
    IWDG->RLR = __IWDG_RLR;	// Init RLR
    IWDG->KR = 0xAAAA;		// Reload the watchdog
    IWDG->KR = 0xCCCC;		// Start the watchdog
    } /***** IwdgSetup() *****/

void IwdgRestart(void)
    {
    IWDG->KR = 0xAAAA;
    }/***** IwdgRestart() *****/

void InitSMS(void)
    {
    unsigned int cnt;
    rBuf[0] = 193;
    rBuf[1] = 105;
    rBuf[2] = 74;
    rBuf[3] = 59;	//IP adresa INFOBIP servera
    rBuf[4] = 0;
    rBuf[5] = 80;	//HTTP port
    EEwrite(EE_GSM_IP, &rBuf[0], 6);

    strcpy((char*) &rBuf[0], "admin");
    cnt = strlen("admin");
    rBuf[cnt] = 0;
    EEwrite(EE_GSM_UNAME, &rBuf[0], cnt + 1);

    strcpy((char*) &rBuf[0], "admin");
    cnt = strlen("admin");
    rBuf[cnt] = 0;
    EEwrite(EE_GSM_PASS, &rBuf[0], cnt + 1);

    strcpy((char*) &rBuf[0], "0911234567");
    cnt = strlen("0911234567");
    rBuf[cnt] = 0;
    EEwrite(EE_GSM1_NUM, &rBuf[0], cnt + 1);

    strcpy((char*) &rBuf[0], "0911234567");
    cnt = strlen("0911234567");
    rBuf[cnt] = 0;
    EEwrite(EE_GSM2_NUM, &rBuf[0], cnt + 1);

    strcpy((char*) &rBuf[0], "0911234567");
    cnt = strlen("0911234567");
    rBuf[cnt] = 0;
    EEwrite(EE_GSM3_NUM, &rBuf[0], cnt + 1);

    strcpy((char*) &rBuf[0], "0911234567");
    cnt = strlen("0911234567");
    rBuf[cnt] = 0;
    EEwrite(EE_GSM4_NUM, &rBuf[0], cnt + 1);

    strcpy((char*) &rBuf[0], "0911234567");
    cnt = strlen("0911234567");
    rBuf[cnt] = 0;
    EEwrite(EE_GSM5_NUM, &rBuf[0], cnt + 1);

    strcpy((char*) &rBuf[0], "ime 1");
    cnt = strlen("ime 1");
    rBuf[cnt] = 0;
    EEwrite(EE_GSM1_NAME, &rBuf[0], cnt + 1);

    strcpy((char*) &rBuf[0], "ime 2");
    cnt = strlen("ime 2");
    rBuf[cnt] = 0;
    EEwrite(EE_GSM2_NAME, &rBuf[0], cnt + 1);

    strcpy((char*) &rBuf[0], "ime 3");
    cnt = strlen("ime 3");
    rBuf[cnt] = 0;
    EEwrite(EE_GSM3_NAME, &rBuf[0], cnt + 1);

    strcpy((char*) &rBuf[0], "ime 4");
    cnt = strlen("ime 4");
    rBuf[cnt] = 0;
    EEwrite(EE_GSM4_NAME, &rBuf[0], cnt + 1);

    strcpy((char*) &rBuf[0], "ime 5");
    cnt = strlen("ime 5");
    rBuf[cnt] = 0;
    EEwrite(EE_GSM5_NAME, &rBuf[0], cnt + 1);

    }/***** InitSMS() *****/

void InitSNTP(void)
    {
    uint8_t cnt;
    strcpy((char*) &rBuf[0], "hr.pool.ntp.org");
    cnt = strlen("hr.pool.ntp.org");
    rBuf[cnt] = 0;
    EEwrite(EE_SNTP1, &rBuf[0], cnt + 1);

    strcpy((char*) &rBuf[0], "at.pool.ntp.org");
    cnt = strlen("at.pool.ntp.org");
    rBuf[cnt] = 0;
    EEwrite(EE_SNTP2, &rBuf[0], cnt + 1);

    EEwrite_8(EE_TIME_ZONE, 1);
    EEwrite_8(EE_SNTP_STATE, 1);
    EEwrite_8(EE_DST_ACTIVETIME, 1);

    }/***** InitSNTP() *****/

void InitSensors(void)
    {
    uint8_t i, cnt;
    uint16_t ee_block;
    uint32_t tmp32;
    for (i = 0; i < 8; i++)
	{
	memset(rBuf, 0, 256);
	rBuf[EE_SENSOR_CODE] = 0;
	rBuf[EE_SENSOR_CODE + 1] = 0;
	rBuf[EE_SENSOR_CODE + 2] = 0;
	rBuf[EE_SENSOR_CODE + 3] = 0;

	rBuf[EE_LOW_ALARM] = 10;	//donja granica alarma
	rBuf[EE_HIGH_ALARM] = 40;	//gornja granica alarma
	rBuf[EE_HIST] = 1;	//histereza
	rBuf[EE_SENSOR_ACTIVE] = 0;	//senzor disable
	rBuf[EE_CHANNEL] = 1;	//kanal
	rBuf[EE_WAPP1_ON] = 0;	//WAPP
	rBuf[EE_WAPP2_ON] = 0;
	rBuf[EE_WAPP3_ON] = 0;
	rBuf[EE_WAPP4_ON] = 0;
	rBuf[EE_WAPP5_ON] = 0;
	rBuf[EE_SMS1] = 0;	//SMS1 OFF
	rBuf[EE_SMS2] = 0;	//SMS1 OFF
	rBuf[EE_SMS3] = 0;	//SMS1 OFF
	rBuf[EE_SMS4] = 0;	//SMS1 OFF
	rBuf[EE_SMS5] = 0;	//SMS1 OFF
	float tKalib = 0;
	memcpy(&rBuf[EE_CALIB], &tKalib, 4);
	rBuf[EE_MEASURE_TYPE] = 0;
	EEwrite(0x100 * (i + 1), rBuf, 64);

	strcpy(&rBuf[0], "Senzor");
	rBuf[0x06] = '1' + i;
	rBuf[0x07] = 0;
	EEwrite((0x100 * (i + 1)) + EE_SENSOR_NAME, rBuf, 32);

	}
    }

/********************************************//**
 * \brief   Inicijalizacija parametara NV RAM-a
 *
 * \param NONE
 * \return NONE
 *
 ***********************************************/
void InitNV(void)
    {
    uint32_t *nvram;
    uint32_t tmp32;
    uint8_t cnt;

    tmp32 = NVRAM_Read32(NV_LOGGER_INIT);
    if (tmp32 != 0x0155AAFE)
	{
	NVRAM_Write32(NV_LOGGER_INIT, 0x0155AAFE);
	//logger srednje vrijednosti nije inicijaliziran, postavlja se u nulu
	for (cnt = 0; cnt < 16; cnt++)
	    NVRAM_Write32(NV_LOGGER_INIT+(cnt*4)+4, 0);
	}

    tmp32 = NVRAM_Read32(NV_MKT_INIT);
    if (tmp32 != 0x0155AAFE)
	{
	NVRAM_Write32(NV_MKT_INIT, 0x0155AAFE);
	//logger MKT mjerenja
	for (cnt = 0; cnt < 8; cnt++)
	    NVRAM_Write32(NV_MKT_COUNT_SEN1+(cnt*4), 0);
	for (cnt = 0; cnt < 160; cnt++)
	    NVRAM_Write32(NV_MKT_SEN1+(cnt*4), 0);
	}
    }/***** InitNV() *****/

void InitParam(void)
    {
    unsigned int cnt;
    unsigned char i = 0;
    uint16_t tmp16, ee_block;
    uint32_t tmp32;
    float tmpfloat;
    tmp16 = EEread_16(EE_INIT);
    if (tmp16 == 0x55AA)
	{
	for (i = 0; i < 8; i++)
	    {
	    ee_block = 0x100 * (i + 1);
	    EEread(0x100 * (i + 1), rBuf, 0x30);
	    memcpy(senzor[i].CODE, rBuf, 8);
	    senzor[i].LOWLIMIT = rBuf[EE_LOW_ALARM];
	    senzor[i].HIGHLIMIT = rBuf[EE_HIGH_ALARM];
	    senzor[i].HIS = rBuf[EE_HIST];

	    senzor[i].ENBLE = rBuf[EE_SENSOR_ACTIVE];
	    senzor[i].KANAL = rBuf[EE_CHANNEL];
	    tmp32 = EEread_32(ee_block + EE_CALIB);
	    memcpy(&tmpfloat, &tmp32, 4);
	    senzor[i].kalibracija = tmpfloat;
	    }
	EEread(EE_PASS, password, 16);
	EEread(EE_USERNAME, username, 16);
	scansetup = EEread_16(EE_SCAN_TIME);
	sntp.dston = EEread_8(EE_DST_STATE);
	sntp.sntpon = EEread_8(EE_SNTP_STATE);

	}
    else
	{
	EEwrite_16(EE_INIT, 0x55AA);
	EEwrite_32(EE_IP, 0xC0A801C8);
	EEwrite_32(EE_SUBNET, 0xFFFFFF00);
	EEwrite_32(EE_GATEWAY, 0xC0A80101);
	EEwrite_32(EE_DNS, 0x08080808);
	EEwrite_16(EE_HTTP_PORT, 80);
	EEwrite_8(EE_DDNS_STATE, 0);
	EEwrite_8(EE_DHCP_STATE, 0);

	strcpy(rBuf, DEFAULT_DDNS_HOST);
	EEwrite(EE_DDNS_HOST, rBuf, 32);
	strcpy(rBuf, DEFAULT_NAZIV_OBJEKTA);
	EEwrite(EE_OBJECT_NAME, rBuf, 32);
	//inicijalizacija passworda
	strcpy(rBuf, "s");
	EEwrite(EE_PASS, rBuf, 16);
	EEwrite(EE_USERNAME, rBuf, 16);
	strcpy(username, "s");
	strcpy(password, "s");

	//scan time
	EEwrite_16(EE_SCAN_TIME, 15);

	//upis 8 senzora
	InitSensors();
	InitSMS();
	InitSNTP();

	}

    }/***** InitParam() *****/
void LEDInit(void)
    {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    LED_OFF();
    }

void InputInit(void)
    {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    }/***** InputInit() ****/

/**
 * @brief  Configure the RTC peripheral by selecting the clock source.
 * @param  None
 * @retval None
 */
void RTC_Config(void)
    {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);/* Enable the PWR clock */

    PWR_BackupAccessCmd(ENABLE);/* Allow access to RTC */
    PWR_BackupRegulatorCmd(ENABLE); /* dozvola napajanja, dodano 19.12.   */
    RCC_LSEConfig(RCC_LSE_ON);/* Enable the LSE OSC */
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)/* Wait till LSE is ready */
	{
	//RCC_LSEConfig(RCC_LSE_ON);/* Enable the LSE OSC */
	}
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);/* Select the RTC Clock Source */
    RCC_RTCCLKCmd(ENABLE);/* Enable the RTC Clock */
    RTC_WaitForSynchro();/* Wait for RTC APB registers synchronisation */
    RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
    RTC_InitStructure.RTC_SynchPrediv = 0xFF;
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
    RTC_Init(&RTC_InitStructure);
    }/***** RTC_Config() *****/

/********************************************************************************
 Function:       CheckScanTime
 Purpose:        Ă�ekira da li je isteklo vrijeme za uzeti uzorak temperature.
 **********************************************************************************/
void CheckScanTime(void)
    {

// TODO (Korisnik#1#): staviti ih kao globalne varijable koje se postave u inicijalizaciji
    uint16_t sms_min, sms_setup_min;
    getDate(&godina, &mjesec, &datum, &dan);
    getTime(&hr, &mn, &sc);
    sms_min = hr * 60 + mn;

    switch (scansetup)
	{
    case 5:
	{
	if (mn == 1 || mn == 6 || mn == 11 || mn == 16 || mn == 21 || mn == 26
		|| mn == 31 || mn == 36 || mn == 41 || mn == 46 || mn == 51
		|| mn == 56)
	    {
	    if (oldmin != mn)
		scanflag = 1;  //isteklo 5 minuta
	    }
	}
	break;
    case 10:
	{
	if (mn == 1 || mn == 11 || mn == 21 || mn == 31 || mn == 41 || mn == 51)
	    {
	    if (oldmin != mn)
		scanflag = 1;  //isteklo 5 minuta
	    }
	}
	break;
    case 15:
	{
	if (mn == 1 || mn == 16 || mn == 31 || mn == 46)
	    {
	    if (oldmin != mn)
		scanflag = 1;  //isteklo 5 minuta
	    }
	}
    case 30:
	{
	if (mn == 1 || mn == 31)
	    {
	    if (oldmin != mn)
		scanflag = 1;
	    }
	}
	break;
    case 60:
	{
	if (oldmin == 0 && mn == 1)
	    scanflag = 1;
	}
	break;
    case 120:
	{
	if (hr == 0 || hr == 2 || hr == 4 || hr == 6 || hr == 8 || hr == 10
		|| hr == 12 || hr == 14 || hr == 16 || hr == 18 || hr == 20
		|| hr == 22)
	    {
	    if (oldmin == 0 && mn == 1)
		scanflag = 1;
	    }
	}
	break;
    case 240:
	{
	if (hr == 0 || hr == 4 || hr == 8 || hr == 12 || hr == 16 || hr == 20)
	    {
	    if (oldmin == 0 && mn == 1)
		scanflag = 1;
	    }
	}
	break;
    case 360:
	{
	if (hr == 0 || hr == 6 || hr == 12 || hr == 18)
	    {
	    if (oldmin == 0 && mn == 1)
		scanflag = 1;
	    }
	}
	break;
	}
    if (sms_status)
	{
//ako je dozvoljeno slanje SMS statusa
	if (sms_min == sms_setup_min)
	    {
	    if (sms_old_min != sms_min)
		SendSMSStatus();	//Ĺˇalje SMS status
	    }
	}

    sms_old_min = sms_min;
    oldmin = mn;
    } /*****CheckScanTime() *****/

/********************************************************************************
 Function:       CheckHaccpScan
 Purpose:        Uzima podatke za HACCP u 6:00,10:00,14:00,18:00 i 22:00 sati.
 **********************************************************************************/
void CheckHaccpScan(void)
    {
    getTime(&hr, &mn, &sc);
    if (mn == 0)
	{
	if (old_haccp_min != mn)
	    {
	    if (hr == 6 || hr == 10 || hr == 14 || hr == 18 || hr == 22)
		SaveHaccp();
	    }
	}
    old_haccp_min = mn;
    } /*****CheckHaccpScan() *****/

/********************************************************************************
 Function:       CheckSNTPScan
 Purpose:        Svaki sat uzima podatak sa SNTP servera i provjerava da li je zimsko/ljetno vrijeme.
 **********************************************************************************/
void CheckSNTPScan(void)
    {
    getTime(&hr, &mn, &sc);
    if (sntpoldhr != hr)
	if (sntp.sntpon == 1)
	    sntp_scan_flag = 1;
    sntpoldhr = hr;
    } /*****CheckSNTPScan() *****/

/********************************************************************************
 Function:       Check4hour
 Purpose:        Svaka 4 sata radi se reset sustava.
 **********************************************************************************/
void Check4hour(void)
    {
    getTime(&hr, &mn, &sc);
    if (mn == 59)
	{
	if (old_reset_min != mn)
	    {
	    if (hr == 1 || hr == 5 || hr == 9 || hr == 13 || hr == 17
		    || hr == 21)
		while (1)
		    {
		    };
	    }
	}
    old_reset_min = mn;
    } /*****Check4hour() *****/

static void zero_fill_bss(void)
    {
    uint8_t *start, *end;

    start = &_sbss;
    end = &_ebss;
    while (start < end)
	*start++ = 0;

    }

static void zero_fill_ccm(void)
    {
    uint8_t *start, *end;

    start = &_sccmram;
    end = &_eccmram;
    while (start < end)
	*start++ = 0;

    }
void lfs_test(void)
    {

    /*    lfs_file_t xfile;
     lfs_file_t yfile;
     lfs_file_t zfile;
     uint8_t xBuf[256];
     uint8_t yBuf[256];
     uint8_t zBuf[256];

     struct lfs_file_config xCfg;
     struct lfs_file_config yCfg;
     struct lfs_file_config zCfg;
     char fBuf[100];
     xCfg.buffer = &xBuf[0];
     yCfg.buffer = &yBuf[0];
     zCfg.buffer = &zBuf[0];
     xCfg.attr_count = 0;
     yCfg.attr_count = 0;
     zCfg.attr_count = 0;

     int err = lfs_file_opencfg(&wtlfs, &xfile, "jure1.txt",
     LFS_O_RDWR | LFS_O_CREAT, &xCfg);
     err = lfs_file_opencfg(&wtlfs, &yfile, "jure2.txt",
     LFS_O_RDWR | LFS_O_CREAT, &yCfg);
     err = lfs_file_opencfg(&wtlfs, &zfile, "jure3.txt",
     LFS_O_RDWR | LFS_O_CREAT, &zCfg);

     strcpy(fBuf, "11111111111111111111111111111111111111111111111111");
     err = lfs_file_write(&wtlfs, &xfile, fBuf, sizeof(fBuf));
     strcpy(fBuf, "222222222222222222222222222222222222222222222222");
     err = lfs_file_write(&wtlfs, &yfile, fBuf, sizeof(fBuf));
     strcpy(fBuf, "333333333333333333333333333333333333333333333333333333");
     err = lfs_file_write(&wtlfs, &zfile, fBuf, sizeof(fBuf));

     err = lfs_file_close(&wtlfs, &xfile);
     err = lfs_file_close(&wtlfs, &yfile);
     err = lfs_file_close(&wtlfs, &zfile);

     err = lfs_file_opencfg(&wtlfs, &xfile, "jure1.txt",
     LFS_O_RDWR | LFS_O_CREAT, &xCfg);
     err = lfs_file_opencfg(&wtlfs, &yfile, "jure2.txt",
     LFS_O_RDWR | LFS_O_CREAT, &yCfg);
     err = lfs_file_opencfg(&wtlfs, &zfile, "jure3.txt",
     LFS_O_RDWR | LFS_O_CREAT, &zCfg);

     err = lfs_file_read(&wtlfs, &xfile, fBuf, 20);
     err = lfs_file_read(&wtlfs, &yfile, fBuf, 15);
     err = lfs_file_read(&wtlfs, &zfile, fBuf, 10);
     err = lfs_file_close(&wtlfs, &xfile);
     err = lfs_file_close(&wtlfs, &yfile);
     err = lfs_file_close(&wtlfs, &zfile);
     */
    }
/**
 * @brief  Inicijalizacija (reinicijalizacija) fajl sistema
 * @param  None
 * @retval Status inicijalizacije
 */
int32_t fsinit(void)
    {
    lfs_format(&wtlfs, &wtcfg);
    int err = lfs_mount(&wtlfs, &wtcfg);
// reformat if we can't mount the filesystem
// this should only happen on the first boot
    if (err)
	{
	lfs_format(&wtlfs, &wtcfg);
	err = lfs_mount(&wtlfs, &wtcfg);
	}

    return err;
    }

void dst_sntp_init(void)
    {
    char sntbuf[40];
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    EEread(EE_SNTP1, &sntbuf[0], 32);
    sntp_setservername(0, &sntbuf[0]);

    EEread(EE_SNTP2, &sntbuf[0], 32);
    sntp_setservername(1, &sntbuf[0]);

    sntp_init();
    }

uint32_t adr32;

//****************************************************
int main(void)
//****************************************************
    {
    int16_t cnt, t;
    int32_t sec_cnt;
    char testarray[64];

    uint32_t adr32;
     struct ip4_addr dns_addr;
//__disable_irq();

//    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0xC000);
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
    zero_fill_bss();	//brisanje memorije
    zero_fill_ccm();
    LEDInit();

    delay_ms(1000);

    I2Cinit();
    InputInit();
    enc424j600Init();

    BootJumperInit();
    ResetJumperInit();

////LFS init---------------------------------------------------
    LFS_Config();
    MX25_Spi_Init();
//ChipErase();
    fsinit();
//lfs_test();
    temp[0] = 0;
    temp[1] = 0;
    temp[2] = 0;
    temp[3] = 0;
    temp[4] = 0;
    temp[5] = 0;
    temp[6] = 0;
    temp[7] = 0;

    InitParam();

    LwIP_Init();
    httpd_init();


    dns_init();
    adr32=EEread_32(EE_DNS);
    IP4_ADDR(&dns_addr,(uint8_t)(adr32>>24), (uint8_t)(adr32>>16), (uint8_t)(adr32>>8), (uint8_t)(adr32));
    dns_setserver(0,&dns_addr);

//WCLinit();
    dst_sntp_init();
    LEDInit();
    LED_OFF();
    LED_ON();
    /* Setup SysTick Timer for 1 msec interrupts  */
    if (SysTick_Config(SystemCoreClock / 1000))
	{
	while (1)
	    ;/* Capture error */
	}

    if ((FLASH_OB_GetBOR() & 0x0F) != OB_BOR_LEVEL3)
	{
	FLASH_Unlock();
	/* Unlocks the option bytes block access */
	FLASH_OB_Unlock();
	/* Clears the FLASH pending flags */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR);
	/* Select The Desired V(BOR) Level------------------------*/
	FLASH_OB_BORConfig(OB_BOR_LEVEL3);
	/* Launch the option byte loading and generate a System Reset */
	FLASH_Lock();
	FLASH_OB_Launch();
	}

    RTC_Config();

    InitNV();   //radi se nakon inicijalizacije RTC-a

    getTime(&hr, &mn, &sc);	//inicijalizacija vremena restarta
    oldmin = mn;
    old_reset_min = mn;
    old_haccp_min = mn;
    scanflag = 0;
    oldhr = hr;
    sntpoldhr = hr;

    in1State = IN1_STATE;
    in2State = IN2_STATE;
    in3State = IN3_STATE;
    in1OldState = in1State;
    in2OldState = in2State;
    in3OldState = in3State;

//----------- WDOG INIT ----------------------------------------------------
//--------------------------------------------------------------------------
IwdgSetup();
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

    __enable_irq();
    delay_ms(1000);
    mqtt_example_init();
    while (1)
	{

	IwdgRestart();
	CheckScanTime();
	Check4hour();
	CheckHaccpScan();
	if (enc424j600IfPacketArrived())
	    LwIP_Pkt_Handle();
	ptToggleLed(&pt_ToggleLed);
	//ptMQTT(&pt_MQTT);
	LwIP_Periodic_Handle(LocalTime);
	ReadTemperature();

//CheckBackup();
//CheckSNTPScan();
	CheckSNTP();

	wcl_poll();
	input_poll();
	logger_alarm_poll();
	mqtt_scan();

	ptArpGratuitous(&pt_ArpGratuitous);
	ptTestLink(&pt_TestLink);

	}
    }/***** main() *****/

PT_THREAD(ptToggleLed(struct pt *pt))
    {

    PT_BEGIN(pt)
    ;
    timer_set(&ledTimer, 1000);
    PT_WAIT_UNTIL(pt, timer_expired(&ledTimer));
    LED_ON();
    timer_set(&ledTimer, 1000);
    PT_WAIT_UNTIL(pt, timer_expired(&ledTimer));
    LED_OFF();
PT_END(pt);
}

/***** ptToggleLed() *****/

PT_THREAD(ptArpGratuitous(struct pt *pt))
{
static struct timer relejTimer;
UINT16 timertmp = 0;
PT_BEGIN(pt)
;
PT_YIELD(pt);
timer_set(&arpTimer, 30000);
PT_WAIT_UNTIL(pt, timer_expired(&arpTimer));
timer_set(&arpTimer, 30000);
PT_WAIT_UNTIL(pt, timer_expired(&arpTimer));
send_myIP(&netif);
timer_set(&arpTimer, 50);
PT_WAIT_UNTIL(pt, timer_expired(&arpTimer));
send_myIP(&netif);
timer_set(&arpTimer, 50);
PT_WAIT_UNTIL(pt, timer_expired(&arpTimer));
send_myIP(&netif);

PT_END(pt);
}/***** ptArpArpGratuitous() *****/

//****************************************************
//ptTestLink(struct pt *pt)
//****************************************************
PT_THREAD(ptTestLink(struct pt *pt))
{
uint8_t linkstatus;
PT_BEGIN(pt)
;
PT_YIELD(pt);
timer_set(&linkTimer, 500);
PT_WAIT_UNTIL(pt, timer_expired(&linkTimer));
linkstatus = enc424j600MACIsLinked();
if (linkstatus == 1)
netif_set_link_up(&netif);
else
netif_set_link_down(&netif);
PT_END(pt);
}/***** ptTestLink() *****/

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *   where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
/* User can add his own implementation to report the file name and line number,
 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

/* Infinite loop */
while (1)
    {
    }
}
#endif

