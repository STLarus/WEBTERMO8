#include "datatypes.h"
#include "define.h"
#include "lfs.h"

struct sensor
    {
	UINT8 CODE[8];	//serijski broj senzora
	INT8 LOWLIMIT;	//donja granica alarma
	INT8 HIGHLIMIT;	//gornja granica alarma
	UINT8 HIS;		//histereza
	UINT8 ENBLE;		//senzor aktivan
	UINT8 STATE;		//0.. NO_ALARM, 1...ALARM_HI, 2...ALARM_LOW
	UINT8 KANAL;
//    char	NAME[32];
	UINT8 STATUS;	//
	UINT8 error_counter;
	UINT16 DELAY;	//kasnjenje alarma
	float kalibracija;
    } senzor[8];

//temperature i stanja senzora
float temp[8];
UINT8 state[8];

#define KALIB_ARRAY_SIZE	21

struct strsms
    {
	char sender[64];		//po�iljalac
	char uname[16];		//username
	char password[16];	//password;
	char gsm1[32];		//telefonski broj
	char gsm2[32];		//telefonski broj
	char gsm3[32];		//telefonski broj
	char gsm4[32];		//telefonski broj
	char gsm5[32];		//telefonski broj
	char gsm1_enable;
	char gsm2_enable;
	char gsm3_enable;
	char gsm4_enable;
	char gsm5_enable;
    } sms;

UINT16 logPos;
UINT8 logType;
char logBuf[512];
BYTE SetupNAME[33];
uint8_t tbuf[80];

int32_t rtn, fd;
UINT fwcnt, frcnt;
UINT16 len;
UINT8 rBuf[1024];
//-------------------

UINT8 sat, minuta, sekunda;
UINT8 dan, mjesec, godina, datum;
char twi_buf[256];

//varijable vezane uz PLC
UINT8 gMonth, gDate, gHour, gMin, gDay, gSec;
UINT16 gYear;

//varijable vezane uz INFOBIP
char infobip_poruka[160];
char infobip_status[32];	//varijabla u koju se sprema status
char infobip_credit[16];	//varijabla za spremanje stanja kredita

UINT8 I2C_error;

//kontrolni glagovi vezani za vrijeme uzorkovanja
char scanflag;	//flag koji signalizira da treba skenirati uzorak

//varijable za PASSWORD
char username[25];
char password[25];
char authen_uri[100];

// varijabla za start backup rutine
char backup_scan_flag;

//varijable vezane za setup skeniranja podataka
uint16_t scansetup;
uint8_t bactmp;
uint8_t sms_status;

// varijabla za sntp_rutine
char sntp_scan_flag;
struct tm *timeinfo;
struct sntpstruct
    {
	UINT8 sntpon;
	UINT8 dston;
    } sntp;

//NVRAM varijable
float *nvSumSen1 = (float*) (NV_SUM_SEN1);
uint32_t *nvCountSen1 = (uint32_t*) (NV_COUNT_SEN1);

uint8_t input1, input2, input3;
uint8_t in1State, in2State, in3State;
uint8_t in1OldState, in2OldState, in3OldState;

// littleFS varijable ------------------------------
lfs_t wtlfs __attribute__ ((section(".ccmram")));
struct lfs_config wtcfg __attribute__ ((section(".ccmram")));

struct lfs_file_config wt_cfg __attribute__ ((section(".ccmram")));
lfs_file_t wt_file __attribute__ ((section(".ccmram")));

struct lfs_file_config http_cfg __attribute__ ((section(".ccmram")));
lfs_file_t http_file __attribute__ ((section(".ccmram")));

struct lfs_file_config tft_cfg __attribute__ ((section(".ccmram")));
lfs_file_t tft_file __attribute__ ((section(".ccmram")));

uint8_t lfsBuf1[256];
uint8_t lfsBuf2[256];
uint8_t lfsBuf3[256];

struct _filetime
    {
	uint16_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
    } filetime;

struct whatsapp
    {
	char Msg[128];
	char enable1;
	char enable2;
	char enable3;
	char enable4;
	char enable5;
    } wapp;

struct digitalinput
    {
	uint8_t enable;	//1..enable;  0..disable
	uint8_t type;	//1..NC;  2... NO
	uint8_t state;	//0...OPEN;  1...CLOSE
	uint8_t alarm;	//0...NOALARM;   1...ALARM
	uint16_t DELAY;	//kasnjenje alarma
    } dinput[3];

