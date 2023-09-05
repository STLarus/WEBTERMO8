#ifndef STRUKTURE_H
#define STRUKTURE_H

#include	"datatypes.h"
#include "define.h"
#include "lfs.h"

#define OFFSET
#define	NO_ALARM	0
#define	ALARM_HI	1
#define	ALARM_LOW 	2

#define NUMBER_OF_SENSOR	8

extern struct sensor
    {
	UINT8 CODE[8];	//serijski broj senzora
	INT8 LOWLIMIT;	//donja granica alarma
	INT8 HIGHLIMIT;	//gornja granica alarma
	UINT8 HIS;		//histereza
	UINT8 ENBLE;		//senzor aktivan
	UINT8 STATE;		//0.. NO_ALARM, 1...ALARM_HI, 2...ALARM_LOW
	UINT8 KANAL;
//char	NAME[32];
	UINT8 STATUS;	//
	UINT8 error_counter;
	UINT16 DELAY;	//kasnjenje alarma
	float kalibracija;
    } senzor[8];

//temperature i stanja senzora
extern float temp[8];
extern UINT8 state[8];
//konstante kalibracije

#define KALIB_ARRAY_SIZE	21

extern struct strsms
    {
	char sender[64];		//po�iljalac
	char uname[16];		//mail adresa 2
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
extern struct pachube_ini
    {
	UINT8 GRAPH_ENABLE;		//graph enable/disable
	UINT32 IP;			//IP adresa servera
	UINT32 FEED;
	UINT8 APIKEY[71];
	UINT16 SEC;		//vrijeme slanja podatka u sekundama
    } pac_ini;

extern UINT16 logPos;
extern UINT8 logType;
extern char logBuf[];
extern BYTE SetupNAME[];
extern char tbuf[];

extern int32_t rtn, fd;
extern UINT fwcnt, frcnt;
extern UINT16 len;
extern UINT8 rBuf[];
//-------------------
//-------------------
extern UINT8 sat, minuta, sekunda;
extern UINT8 dan, mjesec, godina, datum;
extern char twi_buf[];

//varijable vezane uz PLC
extern UINT8 gMonth, gDate, gHour, gMin, gDay, gSec;
extern UINT16 gYear;

//varijable vezane uz INFOBIP
extern char infobip_poruka[];
extern char infobip_status[];	//varijabla u koju se sprema status
extern char infobip_credit[];	//varijabla za spremanje stanja kredita

extern UINT8 I2C_error;

//kontrolni glagovi vezani za vrijeme uzorkovanja
extern char scanflag;	//flag koji signalizira da treba skenirati uzorak

//varijable za PASSWORD
extern char username[25];
extern char password[25];
extern char authen_uri[100];

// varijabla za start backup rutine
extern char backup_scan_flag;

//varijable vezane za setup skeniranja podataka
extern uint16_t scansetup;
extern uint8_t bactmp;
extern uint8_t sms_status;

// varijabla za sntp_rutine
extern char sntp_scan_flag;
extern struct tm *timeinfo;
extern struct sntpstruct
    {
	UINT8 sntpon;
	UINT8 dston;
    } sntp;

//NVRAM varijable
extern float *nvSumSen1;
extern uint32_t *nvCountSen1;

extern uint8_t input1, input2, input3;
extern uint8_t in1State, in2State, in3State;
extern uint8_t in1OldState, in2OldState, in3OldState;

// littleFS varijable ------------------------------
extern lfs_t wtlfs;
extern struct lfs_config wtcfg;

extern struct lfs_file_config wt_cfg;
extern lfs_file_t wt_file;

extern struct lfs_file_config http_cfg;
extern lfs_file_t http_file;

extern struct lfs_file_config tft_cfg;
extern lfs_file_t tft_file;

extern uint8_t lfsBuf1[256];
extern uint8_t lfsBuf2[256];
extern uint8_t lfsBuf3[256];

extern struct _filetime
    {
	uint16_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
    } filetime;

extern struct whatsapp
    {
	char Msg[128];
	char enable1;
	char enable2;
	char enable3;
	char enable4;
	char enable5;
    } wapp;

extern struct digitalinput
    {
	uint8_t enable;	//1..enable;  0..disable
	uint8_t type;	//1..NC;  2... NO
	uint8_t state;	//0...OPEN;  1...CLOSE
	uint8_t alarm;	//0...NOALARM;   1...ALARM
	uint16_t DELAY;	//kasnjenje alarma
    } dinput[];

#endif
