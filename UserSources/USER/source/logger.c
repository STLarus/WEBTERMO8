#include "drajveri.h"
#include "strukture.h"
#include "1wire.h"
#include "define.h"
#include "smtp.h"
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "pt.h"
#include "lfs_tools.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

#define DATA_LOG_SIZE	2000

#define DELTAHR_CONST   80/0.008314472

#define MAIL_BUSY   11
#define MAIL_FREE   12

char dsave[120];
char ttoa[10];
char datname[24];
char tBuf[128];
float mktbuf[20];
char send_mail_flag = 0;

char mailState = MAIL_FREE;

uint8_t wAppStatus, wAppflag;
struct pt pt_Wapp;
uint32_t log_point, logcount_point;

/** \brief  Dodavanje novoe temperature u array. Ako je pun briÅ¡e se najstariji zapis
 *
 * \param   mktlog,      relativna adresa u NVRAM-u poÄ�etka arraya sa logiranim temperaturama, pointer pokazuje na poÃ¨etak arraya
 * \param   *newtemp,   nova temperatura koja se upisuje
 * \param   pcount,   relativna adresa lokacije sa rojem uzoraka (NV_MKT_COUNT_SENx)
 * \return  NONE
 *
 */
void UpdateMKT(uint32_t mktlog, uint32_t pcount, float *newtemp)
    {
    uint8_t cnt;
    uint32_t lgcount = NVRAM_Read32(pcount);
    float tmpfloat;
    if (lgcount < 20)
	{
	NVRAM_WriteFloat(mktlog + (lgcount * 4), *newtemp);
	lgcount++;
	}

    else
	{
	for (cnt = 0; cnt < 19; cnt++)
	    {
	    tmpfloat = NVRAM_ReadFloat(mktlog+(cnt*4)+4);
	    NVRAM_WriteFloat(mktlog + (cnt * 4), tmpfloat);
	    }
	NVRAM_WriteFloat(mktlog + (cnt * 4), *newtemp);
	}
    NVRAM_Write32(pcount, lgcount);
    }/***** UpdateMKT() *****/

/** \brief  IzraÃ¨un MKT temperature
 *
 * \param   mktlog,   adresa u NVRAM-u
 * \param   ount,   broj uzoraka u log bufferu
 * \param   rettemp,   pointer na varijablu u koju se upisuje MKT temperatura
 * \return  NONE
 *
 */
void CalculateMKT(uint32_t mktlog, uint32_t count_adr, float *rettemp)
    {
    uint8_t cnt;
    uint32_t logcount;
    float tfloat, ftemp, fsum;
    logcount = NVRAM_Read32(count_adr);
    fsum = 0;
    for (cnt = 0; cnt < logcount; cnt++)
	{
	ftemp = NVRAM_ReadFloat(mktlog);
	mktlog += 4;
	tfloat = 80 / (0.008314472 * (ftemp + 273));   //izraÃ¨un RT u kelvinima
	tfloat = exp(0 - tfloat);  //e to the negative deltaH/RT
	fsum += tfloat;
	}
    fsum = fsum / logcount;    //dijeli se sa brojem uzoraka
    ftemp = log(fsum);        //negativni logaritam od sume
    ftemp = 0 - ftemp;
    fsum = DELTAHR_CONST / ftemp;  //MKT Calculation degree K
    ftemp = fsum - 273;    //pretvara iz kelvina u celzijuse
    *rettemp = ftemp;
    }/***** CalculateMKT() *****/

void CreateDatName(char *fname, UINT8 god, UINT8 mjes)
    {

    uint8_t frval;
    //Ime fajla je T0408.CSV; 04 je misec, 08 je godin
    strcpy(fname, "T");
    if (mjes < 10)
	{
	*(fname + 1) = '0';
	*(fname + 2) = mjes + 0x30;
	}
    else
	{
	*(fname + 1) = '1';
	*(fname + 2) = mjes + 0x26;
	}
    sprintf((fname + 3), "%d", god);
    strcat(fname, ".CSV");
    sprintf(tBuf, "MJESEC;%d;GODINA;%d;;", mjes, god + 2000);
    strcat(tBuf,
	    "\r\nDATUM;VRIJEME;SENZOR1;SENZOR2;SENZOR3;SENZOR4;SENZOR5;SENZOR6;SENZOR7;SENZOR8\r\n");

    lfs_create(fname, tBuf);

    }/***** CreateDatName() *****/

void CreateHaccpName(char *fname, UINT8 god, UINT8 mjes)
    {
    strcpy(fname, "H");
    if (mjes < 10)
	{
	*(fname + 1) = '0';
	*(fname + 2) = mjes + 0x30;
	}
    else
	{
	*(fname + 1) = '1';
	*(fname + 2) = mjes + 0x26;
	}
    sprintf((fname + 3), "%d", god);
    strcat(fname, ".CSV");
    sprintf(tBuf, "MJESEC;%d;GODINA;%d;;", mjes, god + 2000);
    strcat(tBuf,
	    "\r\nDATUM;VRIJEME;SENZOR1;SENZOR2;SENZOR3;SENZOR4;SENZOR5;SENZOR6;SENZOR7;SENZOR8\r\n");

    lfs_create(fname, tBuf);
    }/***** CreateHaccpName() *****/

void CreateAlarmName(char *fname, UINT8 god, UINT8 mjes)
    {
    //Ime fajla je A0408.CSV; 04 je misec, 08 je godin
    char tBuf[120];

    UINT frval;
    strcpy(fname, "A");
    if (mjes < 10)
	{
	*(fname + 1) = '0';
	*(fname + 2) = mjes + 0x30;
	}
    else
	{
	*(fname + 1) = '1';
	*(fname + 2) = mjes + 0x26;
	}
    sprintf((fname + 3), "%d", god);
    strcat(fname, ".CSV");

    sprintf(tBuf, "MJESEC;%d;GODINA;%d;", mjes, god + 2000);
    strcat(tBuf, "\r\nDATUM;VRIJEME;SENZOR;TEMPERATURA;TIP\r\n");

    lfs_create(fname, tBuf);
    }/****** CreateAlarmName() *****/

void CreateDigitalAlarmName(char *fname, UINT8 god, UINT8 mjes)
    {
    //Ime fajla je D0408.CSV; 04 je misec, 08 je godin
    char tBuf[120];

    UINT frval;
    strcpy(fname, "A");
    if (mjes < 10)
	{
	*(fname + 1) = '0';
	*(fname + 2) = mjes + 0x30;
	}
    else
	{
	*(fname + 1) = '1';
	*(fname + 2) = mjes + 0x26;
	}
    sprintf((fname + 3), "%d", god);
    strcat(fname, ".CSV");

    sprintf(tBuf, "MJESEC;%d;GODINA;%d;", mjes, god + 2000);
    strcat(tBuf, "\r\nDATUM;VRIJEME;INPUT\r\n");

    lfs_create(fname, tBuf);
    }/****** CreateDigitalAlarmName() *****/

void SaveTemp(void)
    {
    uint8_t cnt;
    uint16_t frval;
    float ftmp;
    uint32_t tcount;
    uint8_t mktstate;
    log_point = (*(__IO uint32_t*) (BKPSRAM_BASE + NV_MKT_SEN1));
    logcount_point = (*(__IO uint32_t*) (BKPSRAM_BASE + NV_MKT_COUNT_SEN1));
    getTime(&sat, &minuta, &sekunda);
    getDate(&godina, &mjesec, &datum, &dan);
    CreateDatName(&datname[0], godina, mjesec);

    sprintf(dsave, "%d;%d:%d:%d;", datum, sat, minuta, sekunda);
    ftmp = *nvSumSen1;
    tcount = *nvCountSen1;
    ftmp = ftmp / tcount;
    *nvSumSen1 = 0;
    *nvCountSen1 = 0;
    temp[0] = ftmp;

    for (cnt = 0; cnt < (NUMBER_OF_SENSOR); cnt++)
	{
	if (senzor[cnt].ENBLE == 1)
	    {
	    //izraÄ�un prosjeÄ�ne temperature
	    ftmp = NVRAM_ReadFloat(NV_SUM_SEN1+(cnt*8));
	    tcount = NVRAM_Read32(NV_COUNT_SEN1+(cnt*8));
	    if (tcount != 0)
		{
		ftmp = ftmp / tcount;
		NVRAM_WriteFloat(NV_SUM_SEN1+(cnt*8), 0);
		NVRAM_Write32(NV_COUNT_SEN1+(cnt*8), 0);
		temp[cnt] = ftmp;

		UpdateMKT(NV_MKT_SEN1 + (80 * cnt),
		NV_MKT_COUNT_SEN1 + (cnt * 4), &temp[cnt]); //dodaje novu temperaturu u MKT buffer

		mktstate = EEread_8(0x100 * (cnt + 1) + EE_MEASURE_TYPE);
		if (mktstate == 1)
		    CalculateMKT(NV_MKT_SEN1 + (80 * cnt),
		    NV_MKT_COUNT_SEN1 + (cnt * 4), &temp[cnt]); //MKT temperatura
		}

	    temp_to_ascii(temp[cnt], ttoa);
	    strcat(dsave, ttoa);
	    }
	strcat(dsave, ";");
	}
    strcat(dsave, "\r\n");
    lfs_append(datname, dsave, strlen(dsave));
    }/***** SaveTemp() *****/

void SaveHaccp(void)
    {
    char cnt;
    float ftmp;
    uint32_t tcount;
    uint8_t mktstate;

    getTime(&sat, &minuta, &sekunda);
    getDate(&godina, &mjesec, &datum, &dan);
    CreateHaccpName(&datname[0], godina, mjesec);
    sprintf(dsave, "%d;%d:%d:%d;", datum, sat, minuta, sekunda);
    ftmp = *nvSumSen1;
    tcount = *nvCountSen1;
    ftmp = ftmp / tcount;
    *nvSumSen1 = 0;
    *nvCountSen1 = 0;
    temp[0] = ftmp;

    for (cnt = 0; cnt < (NUMBER_OF_SENSOR); cnt++)
	{
	if (senzor[cnt].ENBLE == 1)
	    {
	    //izraÄ�un prosjeÄ�ne temperature
	    ftmp = NVRAM_ReadFloat(NV_SUM_SEN1+(cnt*8));
	    tcount = NVRAM_Read32(NV_COUNT_SEN1+(cnt*8));
	    if (tcount != 0)
		{
		ftmp = ftmp / tcount;
		NVRAM_WriteFloat(NV_SUM_SEN1+(cnt*8), 0);
		NVRAM_Write32(NV_COUNT_SEN1+(cnt*8), 0);
		temp[cnt] = ftmp;

		mktstate = EEread_8(0x100 * (cnt + 1) + EE_MEASURE_TYPE);
		if (mktstate == 1)
		    CalculateMKT(NV_MKT_SEN1 + (80 * cnt),
		    NV_MKT_COUNT_SEN1 + (cnt * 4), &temp[cnt]); //MKT temperatura
		}

	    temp_to_ascii(temp[cnt], ttoa);
	    strcat(dsave, ttoa);
	    }
	strcat(dsave, ";");
	}
    strcat(dsave, "\r\n");
    lfs_append(datname, dsave, strlen(dsave));
    }/***** SaveHaccp() *****/

SendWApp(UINT8 broj_senzora)
//void SendWApp(float TAlarm, UINT8 broj_senzora, UINT8 type)
    {
    char ttoa[10];
    uint8_t type;
    float TAlarm = temp[broj_senzora];
    if (TAlarm > senzor[broj_senzora].HIGHLIMIT)
	type = ALARM_HI;
    else if (TAlarm < senzor[broj_senzora].LOWLIMIT)
	type = ALARM_LOW;
    else
	type = NO_ALARM;

    EEread(EE_OBJECT_NAME, rBuf, 64);
    strcpy(wapp.Msg, (char const*) rBuf);	// naziv objekta
    strcat(wapp.Msg, "\n");

    broj_senzora++;
    EEread((broj_senzora * 0x100), rBuf, 256);

    strcat(wapp.Msg, (char const*) &rBuf[EE_SENSOR_NAME]);	// naziv senzora
    strcat(wapp.Msg, "\n");

    if (type == ALARM_HI)
	{
	strcat(wapp.Msg, " ALARM VISOKE TEMPERATURE \r");
	temp_to_ascii(TAlarm, ttoa);
	strcat(wapp.Msg, ttoa);
	strcat(wapp.Msg, " C\n");
	}
    else if (type == ALARM_LOW)
	{
	strcat(wapp.Msg, " ALARM NISKE TEMPERATURE \r");
	temp_to_ascii(TAlarm, ttoa);
	strcat(wapp.Msg, ttoa);
	strcat(wapp.Msg, " C\n");
	}
    wAppflag = 0;
    wapp.enable1 = 0;
    wapp.enable2 = 0;
    wapp.enable3 = 0;
    wapp.enable4 = 0;
    wapp.enable5 = 0;
    if (rBuf[EE_WAPP1_ON])
	{
	wAppflag = 1;
	wapp.enable1 = 1;
	}
    if (rBuf[EE_WAPP2_ON])
	{
	wAppflag = 1;
	wapp.enable2 = 1;
	}
    if (rBuf[EE_WAPP3_ON])
	{
	wAppflag = 1;
	wapp.enable3 = 1;
	}
    if (rBuf[EE_WAPP4_ON])
	{
	wAppflag = 1;
	wapp.enable4 = 1;
	}
    if (rBuf[EE_WAPP5_ON])
	{
	wAppflag = 1;
	wapp.enable5 = 1;
	}
    }/***** SendWApp() *****/

SendWAppInput(UINT8 broj_senzora)
    {
    char ttoa[10];
    uint8_t cnt = broj_senzora - 1;
    uint8_t type;

    EEread(EE_OBJECT_NAME, rBuf, 64);
    strcpy(wapp.Msg, (char const*) rBuf);	// naziv objekta
    strcat(wapp.Msg, "\n");

    EEread(EE_INPUT_NAME1 + (broj_senzora * 0x20), rBuf, 30);
    strcat(wapp.Msg, (char const*) rBuf);	// naziv objekta
    strcat(wapp.Msg, "\n");
    strcat(wapp.Msg, " ALARM  \r");

    EEread(EE_INPUT_WAPP1, rBuf, 5);
    wAppflag = 0;
    wapp.enable1 = 0;
    wapp.enable2 = 0;
    wapp.enable3 = 0;
    wapp.enable4 = 0;
    wapp.enable5 = 0;
    if (rBuf[0])
	{
	wAppflag = 1;
	wapp.enable1 = 1;
	}
    if (rBuf[1])
	{
	wAppflag = 1;
	wapp.enable2 = 1;
	}
    if (rBuf[2])
	{
	wAppflag = 1;
	wapp.enable3 = 1;
	}
    if (rBuf[3])
	{
	wAppflag = 1;
	wapp.enable4 = 1;
	}
    if (rBuf[4])
	{
	wAppflag = 1;
	wapp.enable5 = 1;
	}
    }/***** SendWApp() *****/

void SendSMS(uint8_t broj_senzora, uint8_t tipalarma)
    {
    char ttoa[10];
    char sms1, sms2, sms3, sms4, sms5;
    uint8_t cnt = broj_senzora - 1;
    uint8_t type;

    if (tipalarma == 1)
	{	//ALARM senzora temperature
	broj_senzora++;
	EEread(0x100 * (broj_senzora) + EE_SENSOR_NAME, rBuf, 16);

	float TAlarm = temp[broj_senzora - 1];
	if (TAlarm > senzor[cnt].HIGHLIMIT)
	    type = ALARM_HI;
	else if (TAlarm < senzor[cnt].LOWLIMIT)
	    type = ALARM_LOW;
	else
	    type = NO_ALARM;
//    READ_FLASH(broj_senzora*0x100,rBuf,256);
	strcpy(infobip_poruka, (char const*) &rBuf[0]);	// naziv senzora
	strcat(infobip_poruka, "\r\n");
	if (type == ALARM_HI)
	    {
	    strcat(infobip_poruka, " ALARM VISOKE TEMPERATURE ");
	    temp_to_ascii(TAlarm, ttoa);
	    strcat(infobip_poruka, ttoa);
	    strcat(infobip_poruka, " C\r\n");
	    }
	else if (type == ALARM_LOW)
	    {
	    strcat(infobip_poruka, " ALARM NISKE TEMPERATURE \r\n");
	    temp_to_ascii(TAlarm, ttoa);
	    strcat(infobip_poruka, ttoa);
	    strcat(infobip_poruka, " C\r\n");
	    }
	else if (type == ALARM_ERR)
	    strcat(infobip_poruka, " GRESKA OCITANJA SENZORA \r\n");
	EEread(0x100 * (broj_senzora) + EE_SMS1, rBuf, 5);
	sms1 = rBuf[0x00];
	sms2 = rBuf[0x01];
	sms3 = rBuf[0x02];
	sms4 = rBuf[0x03];
	sms5 = rBuf[0x04];
	send_infobip(sms1, sms2, sms3, sms4, sms5);
	}
    else if (tipalarma == 2)	//ALARM digitalnih ulaza
	{
	EEread(EE_INPUT_NAME1 + (broj_senzora * 0x20), rBuf, 30);
	strcpy(infobip_poruka, (char const*) &rBuf[0]);	// naziv senzora
	strcat(infobip_poruka, "\r\n");
	strcat(infobip_poruka, " ALARM ");
	strcat(infobip_poruka, "\r\n");

	EEread(EE_INPUT_SMS1, rBuf, 5);
	sms1 = rBuf[0x00];
	sms2 = rBuf[0x01];
	sms3 = rBuf[0x02];
	sms4 = rBuf[0x03];
	sms5 = rBuf[0x04];
	send_infobip(sms1, sms2, sms3, sms4, sms5);
	}

    }/***** SendSMS() *****/

void SendSMSStatus(void)
    {
    char ttoa[10], k, brsen;
    char sen_name[32];
    char sms1, sms2, sms3, sms4, sms5;
    *infobip_poruka = 0;
    sms1 = 0;
    sms2 = 0;
    sms3 = 0;
    sms4 = 0;
    sms5 = 0;
//temp_to_ascii(temp[cnt],ttoa);
    for (k = 0; k < NUMBER_OF_SENSOR; k++)
	{
	if (senzor[k].ENBLE)
	    {
	    brsen = k + 1;
//            READ_FLASH(brsen*0x100,rBuf,256);
	    strcpy(sen_name, (char const*) &rBuf[0x20]);	// naziv senzora
	    if (strlen(sen_name) > 8)
		sen_name[8] = 0;//uzima prvih osam karaktera od imena zbog maximalne duÅ¾ine poruke
	    strcat(sen_name, " ");
	    temp_to_ascii(temp[k], ttoa);
	    strcat(sen_name, ttoa);
	    strcat(sen_name, " C\r\n");
	    //strcat(sen_name," C  ");
	    strcat(infobip_poruka, sen_name);
	    if (rBuf[0x63] == 1)
		sms1 = 1;
	    if (rBuf[0x64] == 1)
		sms2 = 1;
	    if (rBuf[0x65] == 1)
		sms3 = 1;
	    if (rBuf[0x66] == 1)
		sms4 = 1;
	    if (rBuf[0x67] == 1)
		sms5 = 1;
	    }
	}
    send_infobip(sms1, sms2, sms3, sms4, sms5);
    }/***** SendSMSStatus() *****/

void SaveAlarm(float TAlarm, UINT8 sensor_num, UINT8 type)
    {
    char dsave[100];
    char ttoa[10];
    UINT8 tipalarma = type;
    UINT frval;
//SendWApp(TAlarm, sensor_num, type);
//SendSMS(TAlarm, sensor_num, type);
    getTime(&sat, &minuta, &sekunda);
    getDate(&godina, &mjesec, &datum, &dan);
    CreateAlarmName(&datname[0], godina, mjesec);
    sprintf(dsave, "%d;%d:%d:%d;", datum, sat, minuta, sekunda);
    sensor_num++;
    EEread(sensor_num * 0x100 + EE_SENSOR_NAME, rBuf, 30);
    strcat(dsave, (char const*) &rBuf[0]);
    strcat(dsave, ";");	// naziv senzora
    temp_to_ascii(TAlarm, ttoa);
    strcat(dsave, ttoa);
    strcat(dsave, ";");
    if (type == ALARM_HI)
	strcat(dsave, "VISOKA TEMPERATURA;");
    else if (type == ALARM_LOW)
	strcat(dsave, "NISKA TEMPERATURA;");
    strcat(dsave, "\r\n");
    lfs_append(datname, dsave, strlen(dsave));

    }/***** SaveAlarm() *****/

void SaveDigitalAlarm(uint8_t sensor_num)
    {
    char dsave[100];
    getTime(&sat, &minuta, &sekunda);
    getDate(&godina, &mjesec, &datum, &dan);
    CreateAlarmName(&datname[0], godina, mjesec);
    sprintf(dsave, "%d;%d:%d:%d;", datum, sat, minuta, sekunda);
    sensor_num++;
    EEread(sensor_num * 0x100 + EE_INPUT_NAME1, rBuf, 30);
    strcat(dsave, (char const*) &rBuf[0]);
    strcat(dsave, ";");	// naziv senzora

    strcat(dsave, "\r\n");
    lfs_append(datname, dsave, strlen(dsave));

    }/***** SaveAlarm() *****/

PT_THREAD(ptWapp(struct pt *pt))
    {
    uint32_t local_ip, u32tmp;
    PT_BEGIN(pt)
    ;
    PT_YIELD(pt);
    char tbuf1[64];
    char tbuf2[64];

    PT_WAIT_UNTIL(pt, wAppStatus == WAPP_FREE);

    if (wapp.enable1)
	{
	wAppStatus = WAPP_BUSY;
	send_whatsapp_message(1, wapp.Msg);
	}
    PT_WAIT_UNTIL(pt, wAppStatus == WAPP_FREE);

    if (wapp.enable2)
	{
	wAppStatus = WAPP_BUSY;
	send_whatsapp_message(2, wapp.Msg);
	}
    PT_WAIT_UNTIL(pt, wAppStatus == WAPP_FREE);

    if (wapp.enable3)
	{
	wAppStatus = WAPP_BUSY;
	send_whatsapp_message(3, wapp.Msg);
	}
    PT_WAIT_UNTIL(pt, wAppStatus == WAPP_FREE);

    if (wapp.enable4)
	{
	wAppStatus = WAPP_BUSY;
	send_whatsapp_message(4, wapp.Msg);
	}
    PT_WAIT_UNTIL(pt, wAppStatus == WAPP_FREE);

    if (wapp.enable5)
	{
	wAppStatus = WAPP_BUSY;
	send_whatsapp_message(5, wapp.Msg);
	}
    PT_WAIT_UNTIL(pt, wAppStatus == WAPP_FREE);

    wAppflag = 0;
PT_END(pt);
}/***** ptWapp() *****/

void logger_alarm_poll(void)
{
if (wAppflag == 1)
    ptWapp(&pt_Wapp);

}
