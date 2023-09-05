#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "strukture.h"
#include "datatypes.h"
#include "define.h"
#include "sys_ini.h"
#include "system.h"
#include "drajveri.h"
#include "smtp.h"
#include "define.h"
#include "http_client.h"
#include "stm32f4xx.h"
#include "lfs.h"
char tb[130];
char tmp[40];
unsigned char pb[40];
char *iBuf;
UINT8 *inDataBuf;

//char SVABuf[SVA_BUF_SIZE] __attribute__ ((section(".ccmram")));

/********* varijable vezane uz bootloader ******/
#define BOOT_FLASH_FIRST_PAGE_ADDRESS 0x08000000
typedef void (*pFunction)(void);
pFunction Jump_To_Application;
u32 JumpAddress;

extern unsigned char https_read_encoded2(char*);
extern void my_smtp_result_fn(void *arg, u8_t smtp_result, u16_t srv_err,
	err_t err);
extern unsigned int pacWIDTH, pacHEIGHT, pacGRAPH;
extern char file_name[];

extern lfs_t lfs;
extern lfs_file_t file;
extern uint8_t wAppflag;
/****************************************************************
 ASCII kod tastera pretvara u array
 *****************************************************************/
unsigned char DS18B20CodeToArray(UINT8 *ds_code, unsigned char inLen)
    {
    UINT8 hcod;
    char *tPoint;
    tPoint = iBuf + inLen;
    while (iBuf < tPoint) // traži kraj stringa
	{
	hcod = asciitohex(*iBuf++);
	hcod = hcod << 4;
	hcod = hcod + asciitohex(*iBuf++);
	*ds_code++ = hcod;
	}
    return 1;
    }/***** DS18B20CodeToArray() *****/

/****************************************************************
 Traži string u pristigloj poruci i upisuje ga u array od strukture
 *****************************************************************/
unsigned char StringToFile(UINT8 *retBuf, unsigned char inLen)
    {
    unsigned char a, cnt = 0;
    char *tPoint;
    tPoint = iBuf + inLen;
    cnt = 0;
    while (iBuf < tPoint) // traži kraj stringa
	{
	if (isalnum(*iBuf))
	    tb[cnt++] = *iBuf++;
	else if (isspace(*iBuf))
	    tb[cnt++] = *iBuf++;
	else if ((*iBuf) == '-' || (*iBuf) == '_' || (*iBuf) == '.'
		|| (*iBuf) == '@' || (*iBuf) == '?' || (*iBuf) == '#')
	    tb[cnt++] = *iBuf++;
	else if ((*iBuf++) == '%')
	    {
	    //naletio je na ESC sekvencu
	    a = https_read_encoded2(iBuf);
	    iBuf += 2;
	    if (a == 0x20) //trazi space i upisuje samo njega
		tb[cnt++] = a; //upisuje SPACE
	    }
	else
	    iBuf++;
	if (cnt > inLen)
	    return 0;
	}
    tb[cnt] = 0;
    iBuf++;
    strcpy((char*) retBuf, &tb[0]);
    return cnt;
    }/***** StringToFile() *****/

/****************************************************************
 Pretvara ascii karaktere u jedan bajt i vraca ga
 *****************************************************************/
unsigned char getChar(void)
    {
    unsigned char cnt = 0, eVal;
    while (*iBuf != '&')
	{
	if (isdigit(*iBuf))
	    tb[cnt++] = *iBuf++;
	else
	    return 0;
	}
    tb[cnt] = 0;
    iBuf++;
    eVal = (unsigned char) atoi(&tb[0]);
    return eVal;
    }/***** getChar() *****/

/****************************************************************
 Pretvara ascii karaktere temperature u jedan bajt i vraca ga
 *****************************************************************/
char getTemp(unsigned char inLen)
    {
    char *tPoint;
    unsigned char cnt = 0, eVal;
    char minus_flag = 0;

    tPoint = iBuf + inLen;
    while (iBuf < tPoint) // traži kraj stringa
	{
	if ((*iBuf) == '-')
	    {
	    //negativna temperatura
	    minus_flag = 1;
	    iBuf++;
	    }
	if (isdigit(*iBuf))
	    tb[cnt++] = *iBuf++;
	else
	    return 0;
	}
    tb[cnt] = 0;
    iBuf++;
    eVal = (unsigned char) atoi(&tb[0]);
    if (minus_flag)
	eVal = 0xFF - eVal + 1;
    return eVal;
    }/***** getTemp() *****/

/***************************************************************************
 Pretvara ascii karaktere u jedan bajt i upisuje ih strukturu za upis u fajl
 ***************************************************************************/
unsigned char CharToFile(UINT8 *Fadr)
    {
    unsigned char eVal;
    eVal = getChar();
    *Fadr = eVal;
    return 1;
    }/***** CharToFile() *****/

/***********************************************************************
 Pretvara niz od maximalno 5 karaktera u dva bajta i vraca ih
 ***********************************************************************/
unsigned int getInt(void)
    {
    unsigned char cnt = 0;
    unsigned int eVal;
    while (*iBuf != '&')
	{
	if (isdigit(*iBuf))
	    tb[cnt++] = *iBuf++;
	else
	    return 0;
	}
    tb[cnt] = 0;
    iBuf++;
    eVal = (unsigned int) atoi(&tb[0]);
    return eVal;
    }/***** getInt() *****/

/****************************************************************************
 Pretvara niz od maximalno 5 karaktera u dva bajta i upisuje ih u strukturu
 ****************************************************************************/
unsigned int IntToFile(UINT8 *retBuf, UINT8 inLen)
    {
    unsigned int eVal;
    char *tPoint;
    char intBuf[6], cnt;
    tPoint = iBuf + inLen;
    cnt = 0;
    while (iBuf < tPoint)
	{
	//traži do kraja stringa
	if (isdigit(*iBuf))
	    intBuf[cnt++] = *iBuf++;
	else
	    return 0;
	}
    intBuf[cnt] = 0;
    eVal = (unsigned int) atoi(&intBuf[0]);
    *retBuf++ = eVal / 256;
    *retBuf = eVal % 256;
    return 1;
    }/***** IntToFile() *****/

/****************************************************************
 Funkcija zapis tipa 192.168.1.200 pretvara u 4 bajta
 retBuf..............pointer na buffer u koji se sprema pronaðeni tag
 inLen...............dužina pronaðenog stringa
 return..........0   zapis nije ispravan
 1	pronašao ispravan zapis
 *****************************************************************/
unsigned char IPtoFile(UINT8 *retBuf, UINT8 inLen)
    {
    uint8_t ct[5], a;
    unsigned int k;
    char *tPoint;
    tPoint = iBuf + inLen;	//zadnja lokacija do koje se traži string
    for (a = 0; a < 5; a++)
	ct[a] = 0;
    a = 0;
    while (*iBuf != '.')
	{
	if (isdigit(*iBuf))
	    ct[a++] = *iBuf++;
	else
	    return 0;
	}
    iBuf++; //preskace tocku
    k = atoi(&ct[0]);
    *retBuf++ = k;
    for (a = 0; a < 5; a++)
	ct[a] = 0;
    a = 0;
    while (*iBuf != '.')
	{
	if (isdigit(*iBuf))
	    ct[a++] = *iBuf++;
	else
	    return 0;
	}
    iBuf++; //preskace tocku
    k = atoi(&ct[0]);
    *retBuf++ = k;
    for (a = 0; a < 5; a++)
	ct[a] = 0;
    a = 0;
    while (*iBuf != '.')
	{
	if (isdigit(*iBuf))
	    ct[a++] = *iBuf++;
	else
	    return 0;
	}
    iBuf++; //preskace tocku
    k = atoi(&ct[0]);
    *retBuf++ = k;
    for (a = 0; a < 5; a++)
	ct[a] = 0;
    a = 0;
    while (iBuf < tPoint)
	{
	//traži do kraja stringa
	if (isdigit(*iBuf))
	    ct[a++] = *iBuf++;
	else
	    return 0;
	}
    k = atoi(&ct[0]);
    *retBuf++ = k;
    return 1;
    }
/***** IPtoFile() *****/

/****************************************************************
 Funkcija zapis tipa 192.168.1.200 pretvara u èetiri bajta i
 vraæa kao long varijablu
 *****************************************************************/
unsigned long IPtoLONG(char *ipBuf)
    {
    uint8_t ct[5], a;
    unsigned int k;
    unsigned long retIP = 0;
    for (a = 0; a < 5; a++)
	ct[a] = 0;
    a = 0;
    while (*ipBuf != '.')
	{
	if (isdigit(*ipBuf))
	    ct[a++] = *ipBuf++;
	else
	    return 0;
	}
    ipBuf++; //preskace tocku
    k = atoi(&ct[0]);
    retIP = k;
    retIP = retIP << 8;
    for (a = 0; a < 5; a++)
	ct[a] = 0;
    a = 0;
    while (*ipBuf != '.')
	{
	if (isdigit(*ipBuf))
	    ct[a++] = *ipBuf++;
	else
	    return 0;
	}
    ipBuf++; //preskace tocku
    k = atoi(&ct[0]);
    retIP = retIP + k;
    retIP = retIP << 8;
    for (a = 0; a < 5; a++)
	ct[a] = 0;
    a = 0;
    while (*ipBuf != '.')
	{
	if (isdigit(*ipBuf))
	    ct[a++] = *ipBuf++;
	else
	    return 0;
	}
    ipBuf++; //preskace tocku
    k = atoi(&ct[0]);
    retIP = retIP + k;
    retIP = retIP << 8;
    for (a = 0; a < 5; a++)
	ct[a] = 0;
    a = 0;
    while (*ipBuf != '&')
	{
	if (isdigit(*ipBuf))
	    ct[a++] = *ipBuf++;
	else if (*ipBuf == '<' || *ipBuf == 0) //ubaceno zbog DYNDNS HTTP protokola
	    break;
	else
	    return 0;
	}
    ipBuf++; //preskace &
    k = atoi(&ct[0]);
    retIP = retIP + k;
    return retIP;
    }
/***** IPtoLONG() *****/

/****************************************************************
 Funkcija pretražuje pristiglu poruku i u njoj pokušava pronaæi
 zadani XML tag. Ako ga naðe vraæa dužinu stringa, ako ne vraæa 0.
 tag...............text string koji se traži
 itagBuf.............pointer na buffer u kojem se nalazi zadani tag
 iBufLen...........maximalna dužina primljene poruke
 rBuf..............pointer na buffer u koji se sprema pronaðeni tag

 return..........(-1)   nije našao tag
 len	dužina pronaðenog stringa
 *****************************************************************/
INT8 XMLParser(char *tag, char *itagBuf, int iBufLen, char *rBuf)
    {
    INT16 rval;
    rval = bufsearch((unsigned char*) itagBuf, iBufLen, (UINT8*) tag);
    if (rval == (-1))
	return (INT8) rval;
    itagBuf += rval + strlen(tag);	//postavlja pointer na poèetak stringa
    rval = 0;
    while ((*itagBuf) != '<')
	{
	//kopira tag dok ne nadje "<"
	*(rBuf++) = *(itagBuf++);
	rval++;
	}
    *rBuf = 0;
    return (INT8) rval;
    }/***** XMLParser *****/

/****************************************************************
 *****************************************************************/
UINT8 SaveOutput(UINT8 *bpoint, UINT16 len)
    {
    return 1;
    }/***** SaveOutput() *****/

/****************************************************************
 *****************************************************************/
UINT8 SaveNet(UINT8 *bpoint, UINT16 len)
    {
    UINT8 varlen;
    uint32_t Adresa;
    inDataBuf = bpoint;
    rBuf[0] = 0xAA;
    rBuf[1] = 0x55;
    /*   varlen = XMLParser("<DHCP>", (char*) inDataBuf, len, tb);
     if (varlen)
     {
     if (!strcmp(tb, "ON"))
     EEwrite_8(EE_DHCP_STATE, 1);
     else
     EEwrite_8(EE_DHCP_STATE, 0);
     }*/
    varlen = XMLParser("<IP>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IPtoFile(pb, varlen))	//-------IP
	    EEwrite(EE_IP, pb, 4);
	}
    varlen = XMLParser("<SUB>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IPtoFile(pb, varlen))	//-------IP
	    EEwrite(EE_SUBNET, pb, 4);
	}
    varlen = XMLParser("<ROUT>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IPtoFile(pb, varlen))	//-------IP
	    EEwrite(EE_GATEWAY, pb, 4);
	}
    varlen = XMLParser("<DNS>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IPtoFile(pb, varlen))	//-------IP
	    EEwrite(EE_DNS, pb, 4);
	}
    varlen = XMLParser("<HTTP>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IntToFile(pb, varlen))	//-------HTTP port
	    EEwrite(EE_HTTP_PORT, pb, 2);
	}
    /*    varlen = XMLParser("<USER>", (char*) inDataBuf, len, tb);
     iBuf = tb;
     if (varlen)
     {
     if (StringToFile(pb, varlen))	//-------USER NAME
     EEwrite(EE_USERNAME, pb, 16);
     }
     varlen = XMLParser("<PASS>", (char*) inDataBuf, len, tb);
     iBuf = tb;
     if (varlen)
     {
     if (StringToFile(pb, varlen))	//-------PASSWORD
     EEwrite(EE_PASS, pb, 16);
     }
     varlen = XMLParser("<HOST>", (char*) inDataBuf, len, tb);
     iBuf = tb;
     if (varlen)
     {
     if (StringToFile(pb, varlen))	//-------DDNS HOST
     EEwrite(EE_DDNS_HOST, pb, 32);
     }*/
    varlen = XMLParser("<NAZIV>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------OBJEKT
	    EEwrite(EE_OBJECT_NAME, pb, 32);
	}
    varlen = XMLParser("<ALARMPORT>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IntToFile(pb, varlen))	//-------ALARM SERVER port
	    EEwrite(EE_ALARM_PORT, pb, 2);
	}
    varlen = XMLParser("<ALARMIP>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IPtoFile(pb, varlen))	//-------ALARM IP
	    EEwrite(EE_ALARM_IP, pb, 4);
	}
    return 1;
    }/***** SaveNet() *****/

/****************************************************************
 *****************************************************************/
UINT8 SaveSms(UINT8 *bpoint, UINT16 len)
    {
    UINT8 varlen;
    UINT16 cnt;
    inDataBuf = bpoint;
    varlen = XMLParser("<IP>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IPtoFile(pb, varlen))	//-------IP
	    EEwrite(EE_GSM_IP, pb, 4);
	}
    varlen = XMLParser("<PORT>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IntToFile(pb, varlen))	//-------PORT
	    EEwrite(EE_GSM_PORT, pb, 2);
	}
    varlen = XMLParser("<UNAME>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------USER NAME
	    EEwrite(EE_GSM_UNAME, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<PASS>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------PASS
	    EEwrite(EE_GSM_PASS, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<GSM1>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------GSM1
	    EEwrite(EE_GSM1_NUM, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<GSM2>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------GSM2
	    EEwrite(EE_GSM2_NUM, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<GSM3>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------GSM3
	    EEwrite(EE_GSM3_NUM, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<GSM4>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------GSM4
	    EEwrite(EE_GSM4_NUM, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<GSM5>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------GSM5
	    EEwrite(EE_GSM5_NUM, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<NAME1>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------GSM1
	    EEwrite(EE_GSM1_NAME, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<NAME2>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------GSM2
	    EEwrite(EE_GSM2_NAME, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<NAME3>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------GSM3
	    EEwrite(EE_GSM3_NAME, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<NAME4>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------GSM4
	    EEwrite(EE_GSM4_NAME, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<NAME5>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------GSM5
	    EEwrite(EE_GSM5_NAME, pb, strlen(pb) + 1);
	}

    return 1;
    }/***** Savesms() *****/

/****************************************************************
 *****************************************************************/
UINT8 SaveWApp(UINT8 *bpoint, UINT16 len)
    {
    UINT8 varlen;
    UINT16 cnt;
    inDataBuf = bpoint;

    varlen = XMLParser("<NUMBER1>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (strlen(iBuf))
	    strcpy(pb, iBuf);
	EEwrite(EE_WAPP1_NUM, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<KEY1>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))
	    EEwrite(EE_WAPP1_KEY, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<NAME1>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))
	    EEwrite(EE_WAPP1_NAME, pb, strlen(pb) + 1);
	}

    varlen = XMLParser("<NUMBER2>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (strlen(iBuf))
	    strcpy(pb, iBuf);
	EEwrite(EE_WAPP2_NUM, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<KEY2>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))
	    EEwrite(EE_WAPP2_KEY, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<NAME2>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))
	    EEwrite(EE_WAPP2_NAME, pb, strlen(pb) + 1);
	}

    varlen = XMLParser("<NUMBER3>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (strlen(iBuf))
	    strcpy(pb, iBuf);
	EEwrite(EE_WAPP3_NUM, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<KEY3>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))
	    EEwrite(EE_WAPP3_KEY, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<NAME3>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))
	    EEwrite(EE_WAPP3_NAME, pb, strlen(pb) + 1);
	}

    varlen = XMLParser("<NUMBER4>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (strlen(iBuf))
	    strcpy(pb, iBuf);
	EEwrite(EE_WAPP4_NUM, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<KEY4>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))
	    EEwrite(EE_WAPP4_KEY, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<NAME4>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))
	    EEwrite(EE_WAPP4_NAME, pb, strlen(pb) + 1);
	}

    varlen = XMLParser("<NUMBER5>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (strlen(iBuf))
	    strcpy(pb, iBuf);
	EEwrite(EE_WAPP5_NUM, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<KEY5>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))
	    EEwrite(EE_WAPP5_KEY, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<NAME5>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))
	    EEwrite(EE_WAPP5_NAME, pb, strlen(pb) + 1);
	}

    return 1;
    }/***** SavWApp() *****/

/****************************************************************
 *****************************************************************/
UINT8 SaveTime(UINT8 *bpoint, UINT16 len)
    {
    uint8_t varlen;
    inDataBuf = bpoint;

    varlen = XMLParser("<HOUR>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	IntToFile((unsigned char*) iBuf, varlen);
	sat = tb[1];
	}
    varlen = XMLParser("<MIN>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	IntToFile((unsigned char*) iBuf, varlen);
	minuta = tb[1];
	}
    varlen = XMLParser("<SEC>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	IntToFile((unsigned char*) iBuf, varlen);
	sekunda = tb[1];
	}
    setTime(sat, minuta, sekunda);
    varlen = XMLParser("<DATE>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	IntToFile((unsigned char*) iBuf, varlen);
	datum = tb[1];
	}
    varlen = XMLParser("<DAY>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	IntToFile((unsigned char*) iBuf, varlen);
	dan = tb[1];
	}
    varlen = XMLParser("<YEAR>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	IntToFile((unsigned char*) iBuf, varlen);
	godina = tb[0] * 256 + tb[1] - 2000;
	}
    varlen = XMLParser("<MONTH>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	IntToFile((unsigned char*) iBuf, varlen);
	mjesec = tb[1];
	}
    setDate(godina, mjesec, datum, dan);
    return 1;
    }/***** SaveTime() *****/

/****************************************************************
 *****************************************************************/
UINT8 DelFile0(UINT8 *bpoint, UINT16 len)
    {
    char tname[32];
    UINT8 varlen;
    inDataBuf = bpoint;
    varlen = XMLParser("<FNAME>", (char*) inDataBuf, len, tname);
    if (varlen)
	{
	strcpy(file_name, tname);
	int retn = lfs_remove(&wtlfs, file_name);
	if (retn < 0)
	    return 0;
	else
	    return 1;
	}
    }/***** DelFile0() ****/

/****************************************************************
 *****************************************************************/
UINT8 DelFile1(UINT8 *bpoint, UINT16 len)
    {
    char tname[64];
    UINT8 varlen;
    inDataBuf = bpoint;
    varlen = XMLParser("<FNAME>", (char*) inDataBuf, len, tname);
    if (varlen)
	{
	strcpy(file_name, tname);
	int retn = lfs_remove(&wtlfs, file_name);
	if (retn < 0)
	    return 0;
	else
	    return 1;
	}
    return 0;
    }/***** DelFile1() ****/

/****************************************************************
 *****************************************************************/
UINT8 Sensor(UINT8 snum, UINT8 *bpoint, UINT16 len)
    {
    UINT8 varlen, cnt;
    UINT16 tint;
    uint8_t sms1, sms2, sms3, sms4, sms5;
    uint8_t wapp1, wapp2, wapp3, wapp4, wapp5;
    uint32_t temp32;
    float tKalib;
    uint16_t ee_block = 0x100 * (snum + 1);
    inDataBuf = bpoint;
    varlen = XMLParser("<NAME>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	StringToFile(rBuf, varlen);	//-------SENSOR NAME
    EEwrite(ee_block + EE_SENSOR_NAME, rBuf, strlen(rBuf) + 1);

    varlen = XMLParser("<SERIALNUM>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	DS18B20CodeToArray(senzor[snum].CODE, varlen);	//--------ADRESA SENZORA
	EEwrite(ee_block + EE_SENSOR_CODE, senzor[snum].CODE, 8);
	}

    varlen = XMLParser("<LOWLIMIT>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	//--------DONJA GRANIÈNA TEMPERATURA
	senzor[snum].LOWLIMIT = getTemp(varlen);
	EEwrite_8(ee_block + EE_LOW_ALARM, senzor[snum].LOWLIMIT);
	}

    varlen = XMLParser("<HIGHLIMIT>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	//--------GORNJA GRANIÈNA TEMPERATURA
	senzor[snum].HIGHLIMIT = getTemp(varlen);
	EEwrite_8(ee_block + EE_HIGH_ALARM, senzor[snum].HIGHLIMIT);
	}

    varlen = XMLParser("<HIST>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	//--------HISTEREZA
	senzor[snum].HIS = getTemp(varlen);
	EEwrite_8(ee_block + EE_HIST, senzor[snum].HIS);
	}
    varlen = XMLParser("<DELAY>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)	//--------ALARM DELAY
	{
	if (IntToFile(pb, varlen))
	    {
	    senzor[snum].DELAY = pb[0] * 256 + pb[1];
	    EEwrite_16(ee_block + EE_DELAY, senzor[snum].DELAY);
	    }
	}
    varlen = XMLParser("<KALIB>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	//--------------KALIBRACIJA
	tKalib = atof(tb);
	senzor[snum].kalibracija = tKalib;
	memcpy(&temp32, &tKalib, 4);
	EEwrite_32(ee_block + EE_CALIB, temp32);
	}
    varlen = XMLParser("<ENABLE>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	//--------------ENABLE
	if (!strcmp(tb, "ON"))
	    senzor[snum].ENBLE = 1;
	else
	    senzor[snum].ENBLE = 0;
	EEwrite_8(ee_block + EE_SENSOR_ACTIVE, senzor[snum].ENBLE);
	}
    varlen = XMLParser("<MTYPE>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	//--------------TIP MJERENJA (MKT ili AVERAGE)
	if (!strcmp(tb, "MKT"))
	    EEwrite_8(ee_block + EE_MEASURE_TYPE, 1);
	else
	    EEwrite_8(ee_block + EE_MEASURE_TYPE, 0);
	}

    inDataBuf = bpoint;
    varlen = XMLParser("<WAPP1>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	//--------------WAPP1
	if (!strcmp(tb, "ON"))
	    wapp1 = 1;
	else
	    wapp1 = 0;
	}

    inDataBuf = bpoint;
    varlen = XMLParser("<WAPP2>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	//--------------MAIL2
	if (!strcmp(tb, "ON"))
	    wapp2 = 1;
	else
	    wapp2 = 0;
	}
    inDataBuf = bpoint;
    varlen = XMLParser("<WAPP3>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	//--------------WAPP3
	if (!strcmp(tb, "ON"))
	    wapp3 = 1;
	else
	    wapp3 = 0;
	}
    inDataBuf = bpoint;
    varlen = XMLParser("<WAPP4>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	//--------------WAPP4
	if (!strcmp(tb, "ON"))
	    wapp4 = 1;
	else
	    wapp4 = 0;
	}
    inDataBuf = bpoint;
    varlen = XMLParser("<WAPP5>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	//--------------WAPP5
	if (!strcmp(tb, "ON"))
	    wapp5 = 1;
	else
	    wapp5 = 0;
	}
    varlen = XMLParser("<KANAL>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	//--------------KANAL
	senzor[snum].KANAL = getTemp(varlen);
	EEwrite_8(ee_block + EE_CHANNEL, senzor[snum].KANAL);
	}
    inDataBuf = bpoint;
    varlen = XMLParser("<SMS1>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	//--------------SMS1
	if (!strcmp(tb, "ON"))
	    sms1 = 1;
	else
	    sms1 = 0;
	}
    varlen = XMLParser("<SMS2>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	//--------------SMS2
	if (!strcmp(tb, "ON"))
	    sms2 = 1;
	else
	    sms2 = 0;
	}
    varlen = XMLParser("<SMS3>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	//--------------SMS3
	if (!strcmp(tb, "ON"))
	    sms3 = 1;
	else
	    sms3 = 0;
	}
    varlen = XMLParser("<SMS4>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	//--------------SMS4
	if (!strcmp(tb, "ON"))
	    sms4 = 1;
	else
	    sms4 = 0;
	}
    varlen = XMLParser("<SMS5>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	//--------------SMS5
	if (!strcmp(tb, "ON"))
	    sms5 = 1;
	else
	    sms5 = 0;
	}
    rBuf[0] = wapp1;
    rBuf[1] = wapp2;
    rBuf[2] = wapp3;
    rBuf[3] = wapp4;
    rBuf[4] = wapp5;

    EEwrite(ee_block + EE_WAPP1_ON, rBuf, 5);

    rBuf[0] = sms1;
    rBuf[1] = sms2;
    rBuf[2] = sms3;
    rBuf[3] = sms4;
    rBuf[4] = sms5;
    EEwrite(ee_block + EE_SMS1, rBuf, 5);

    return 1;
    }/***** Sensor() ****/

/****************************************************************
 *****************************************************************/
UINT8 Sensor1(UINT8 *bpoint, UINT16 len)
    {
    Sensor(0, bpoint, len);
    return 1;
    }/***** Sensor1() ****/

/****************************************************************
 *****************************************************************/
UINT8 Sensor2(UINT8 *bpoint, UINT16 len)
    {
    Sensor(1, bpoint, len);
    return 1;
    }/***** Sensor2() ****/

/****************************************************************
 *****************************************************************/
UINT8 Sensor3(UINT8 *bpoint, UINT16 len)
    {
    Sensor(2, bpoint, len);
    return 1;
    }/***** Sensor3() ****/

/****************************************************************
 *****************************************************************/
UINT8 Sensor4(UINT8 *bpoint, UINT16 len)
    {
    Sensor(3, bpoint, len);
    return 1;
    }/***** Sensor4() ****/

/****************************************************************
 *****************************************************************/
UINT8 Sensor5(UINT8 *bpoint, UINT16 len)
    {
    Sensor(4, bpoint, len);
    return 1;
    }/***** Sensor5() ****/

/****************************************************************
 *****************************************************************/
UINT8 Sensor6(UINT8 *bpoint, UINT16 len)
    {
    Sensor(5, bpoint, len);
    return 1;
    }/***** Sensor6() ****/

/****************************************************************
 *****************************************************************/
UINT8 Sensor7(UINT8 *bpoint, UINT16 len)
    {
    Sensor(6, bpoint, len);
    return 1;
    }/***** Sensor7() ****/

/****************************************************************
 *****************************************************************/
UINT8 Sensor8(UINT8 *bpoint, UINT16 len)
    {
    Sensor(7, bpoint, len);
    return 1;
    }/***** Sensor8() ****/

/****************************************************************
 *****************************************************************/
UINT8 Reset(UINT8 *bpoint, UINT16 len)
    {
    while (1)
	{
	};	//èeka reset od watchdoga
    return 1;
    }/***** Reset() *****/

/****************************************************************
 *****************************************************************/
UINT8 TestSms(UINT8 *bpoint, UINT16 len)
    {
    char tname[64];
    UINT8 varlen;
    *infobip_credit = 0;
    *infobip_status = 0;
    inDataBuf = bpoint;
    varlen = XMLParser("<TSMS>", (char*) inDataBuf, len, tname);
    if (varlen)
	{
	if (!strcmp(tname, "sms1"))
	    {
	    strcpy(infobip_poruka, "SMS1 test poruka");
	    send_infobip(1, 0, 0, 0, 0);
	    }
	else if (!strcmp(tname, "sms2"))
	    {
	    strcpy(infobip_poruka, "SMS2 test poruka");
	    send_infobip(0, 1, 0, 0, 0);
	    }
	else if (!strcmp(tname, "sms3"))
	    {
	    strcpy(infobip_poruka, "SMS3 test poruka");
	    send_infobip(0, 0, 1, 0, 0);
	    }
	else if (!strcmp(tname, "sms4"))
	    {
	    strcpy(infobip_poruka, "SMS4 test poruka");
	    send_infobip(0, 0, 0, 1, 0);
	    }
	else if (!strcmp(tname, "sms5"))
	    {
	    strcpy(infobip_poruka, "SMS5 test poruka");
	    send_infobip(0, 0, 0, 0, 1);
	    }
	else if (!strcmp(tname, "delete"))
	    {
	    //f_chdrive(0);
	    //fresult=f_unlink(LOGMAIL_FNAME);
	    }
	}
    return 0;
    }/***** TestSMS() ****/

/****************************************************************
 *****************************************************************/
UINT8 TestWApp(UINT8 *bpoint, UINT16 len)
    {
    char tname[64];
    UINT8 varlen;
    *infobip_credit = 0;
    *infobip_status = 0;
    inDataBuf = bpoint;
    varlen = XMLParser("<TWAPP>", (char*) inDataBuf, len, tname);
    if (varlen)
	{
	if (!strcmp(tname, "wapp1"))
	    {
	    wAppflag = 1;
	    strcpy(wapp.Msg, "WAPP1 test poruka");
	    wapp.enable1 = 1;
	    }
	else if (!strcmp(tname, "wapp2"))
	    {
	    wAppflag = 1;
	    strcpy(wapp.Msg, "WAPP2 test poruka");
	    wapp.enable2 = 1;
	    }
	else if (!strcmp(tname, "wapp3"))
	    {
	    wAppflag = 1;
	    strcpy(wapp.Msg, "WAPP3 test poruka");
	    wapp.enable3 = 1;
	    }
	else if (!strcmp(tname, "wapp4"))
	    {
	    wAppflag = 1;
	    strcpy(wapp.Msg, "WAPP4 test poruka");
	    wapp.enable4 = 1;
	    }
	else if (!strcmp(tname, "wapp5"))
	    {
	    wAppflag = 1;
	    strcpy(wapp.Msg, "WAPP5 test poruka");
	    wapp.enable5 = 1;
	    }

	}
    return 0;
    }/***** TestWApp() ****/

/****************************************************************
 *****************************************************************/
UINT8 SaveSample(UINT8 *bpoint, UINT16 len)
    {
    UINT8 varlen, i;
    uint16_t stime;
    inDataBuf = bpoint;
    varlen = XMLParser("<CHECK>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    EEwrite_8(EE_STATUS_SEND, 1);
	else
	    EEwrite_8(EE_STATUS_SEND, 0);
	}
    varlen = XMLParser("<REPORT>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{

	if (IntToFile(pb, varlen))
	    {
	    scansetup = pb[0] * 256 + pb[1];
	    EEwrite_16(EE_STATUS_TIME, scansetup);
	    }
	}
    varlen = XMLParser("<LOGTIME>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IntToFile(pb, varlen))
	    {
	    scansetup = pb[0] * 256 + pb[1];
	    EEwrite_16(EE_SCAN_TIME, scansetup);

	    }

	}
    return 1;
    }/***** SaveSample() *****/

/****************************************************************
 *****************************************************************/
UINT8 Pass(UINT8 *bpoint, UINT16 len)
    {
    UINT8 varlen, i;
    inDataBuf = bpoint;
    varlen = XMLParser("<PASS>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	Base64Decode(tb, varlen, pb, 30);
	EEwrite(EE_PASS, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<UNAME>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	Base64Decode(tb, varlen, pb, 30);
	EEwrite(EE_USERNAME, pb, strlen(pb) + 1);
	}

    return 1;
    }/***** Pass() *****/

/****************************************************************
 *****************************************************************/
UINT8 SaveBack(UINT8 *bpoint, UINT16 len)
    {
    UINT8 varlen;
    inDataBuf = bpoint;
    //READ_FLASH(0xF00,rBuf,256);
    varlen = XMLParser("<ONOFF>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))	//-------status ENABLE / DISABLE
	    rBuf[0x08] = 1;
	else
	    rBuf[0x08] = 0;
	}
    varlen = XMLParser("<IP>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IPtoFile(pb, varlen))	//-------IP
	    memcpy(&rBuf[0], pb, 4);
	}
    varlen = XMLParser("<PORT>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IntToFile(pb, varlen))	//-------TFTP port
	    memcpy(&rBuf[4], pb, 2);
	}
    varlen = XMLParser("<BACKTIME>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IntToFile(pb, varlen))	//-------vrijeme aktiviranja backupa
	    memcpy(&rBuf[6], pb, 2);
	(*(__IO uint32_t*) (BKPSRAM_BASE + 5)) = rBuf[6];
	(*(__IO uint32_t*) (BKPSRAM_BASE + 6)) = rBuf[7];
	}
    for (varlen = 0; varlen < 3; varlen++)
	(*(__IO uint32_t*) (BKPSRAM_BASE + 7 + varlen)) = rBuf[8 + varlen];
    //WRITE_FLASH(0xF00,rBuf,256);
    return 1;
    }/***** SaveBack() *****/

/****************************************************************
 *****************************************************************/
UINT8 TestBack(UINT8 *bpoint, UINT16 len)
    {
    char tname[64];
    UINT8 varlen;
    inDataBuf = bpoint;
    varlen = XMLParser("<TEST>", (char*) inDataBuf, len, tname);
    if (varlen)
	backup_scan_flag = 1;
    return 0;
    }/***** TestBack() ****/

/****************************************************************
 *****************************************************************/
UINT8 TestSNTP(UINT8 *bpoint, UINT16 len)
    {
    char tname[64];
    UINT8 varlen;
    inDataBuf = bpoint;
    varlen = XMLParser("<SNTP>", (char*) inDataBuf, len, tname);
    if (varlen)
	sntp_scan_flag = 1;
    return 0;
    }/***** TestSNTP() ****/

/****************************************************************
 *****************************************************************/
UINT8 SaveSNTP(UINT8 *bpoint, UINT16 len)
    {
    uint8_t varlen;
    uint8_t tmp;
    inDataBuf = bpoint;

    varlen = XMLParser("<SNTP1>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------SNTP server 1
	    EEwrite(EE_SNTP1, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<SNTP2>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))	//-------SNTP server 2
	    EEwrite(EE_SNTP2, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<ZONA>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	tmp = (UINT8) atoi(tb);	// vremenska zona
	EEwrite_8(EE_TIME_ZONE, tmp);
	}
    varlen = XMLParser("<SENAB>", (char*) inDataBuf, len, tb);	//ENABLE DISABLE
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    {
	    EEwrite_8(EE_SNTP_STATE, 1);
	    sntp.sntpon = 1;
	    }

	else
	    {
	    EEwrite_8(EE_SNTP_STATE, 0);
	    sntp.sntpon = 0;
	    }
	}

    varlen = XMLParser("<DST>", (char*) inDataBuf, len, tb);	//ENABLE DISABLE
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    {
	    EEwrite_8(EE_DST_STATE, 1);
	    sntp.dston = 1;
	    }

	else
	    {
	    EEwrite_8(EE_DST_STATE, 0);
	    sntp.dston = 0;
	    }

	}
    return 1;
    }/***** SaveSNTP() *****/

/****************************************************************
 *****************************************************************/
UINT8 SaveInput(UINT8 *bpoint, UINT16 len)
    {
    uint8_t varlen;
    uint8_t tmp;
    inDataBuf = bpoint;
    uint8_t input_sms1, input_sms2, input_sms3, input_sms4, input_sms5;
    uint8_t input_wapp1, input_wapp2, input_wapp3, input_wapp4, input_wapp5;

    varlen = XMLParser("<NAME1>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))
	    EEwrite(EE_INPUT_NAME1, pb, strlen(pb) + 1);
	}
    varlen = XMLParser("<NAME2>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))
	    EEwrite(EE_INPUT_NAME2, pb, strlen(pb) + 1);
	}

    varlen = XMLParser("<NAME3>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (StringToFile(pb, varlen))
	    EEwrite(EE_INPUT_NAME3, pb, strlen(pb) + 1);
	}

    varlen = XMLParser("<ALARM1>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    {
	    EEwrite_8(EE_INPUT_ALARM1, 1);
	    dinput[0].enable = 1;
	    }

	else
	    {
	    EEwrite_8(EE_INPUT_ALARM1, 0);
	    dinput[0].enable = 0;
	    }
	}

    varlen = XMLParser("<ALARM2>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    {
	    EEwrite_8(EE_INPUT_ALARM2, 1);
	    dinput[1].enable = 1;
	    }

	else
	    {
	    EEwrite_8(EE_INPUT_ALARM2, 0);
	    dinput[1].enable = 0;
	    }
	}

    varlen = XMLParser("<ALARM3>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    {
	    EEwrite_8(EE_INPUT_ALARM3, 1);
	    dinput[2].enable = 1;
	    }

	else
	    {
	    EEwrite_8(EE_INPUT_ALARM3, 0);
	    dinput[2].enable = 0;
	    }
	}

    varlen = XMLParser("<TIP1>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "NC"))
	    {
	    EEwrite_8(EE_INPUT_TIP1, 1);
	    dinput[0].type = 1;
	    }

	else
	    {
	    EEwrite_8(EE_INPUT_TIP1, 2);
	    dinput[0].type = 2;
	    }
	}
    varlen = XMLParser("<TIP2>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "NC"))
	    {
	    EEwrite_8(EE_INPUT_TIP2, 1);
	    dinput[1].type = 1;
	    }

	else
	    {
	    EEwrite_8(EE_INPUT_TIP2, 2);
	    dinput[1].type = 2;
	    }
	}
    varlen = XMLParser("<TIP3>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "NC"))
	    {
	    EEwrite_8(EE_INPUT_TIP3, 1);
	    dinput[2].type = 1;
	    }

	else
	    {
	    EEwrite_8(EE_INPUT_TIP3, 2);
	    dinput[2].type = 2;
	    }
	}

    varlen = XMLParser("<WAPP1>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    input_wapp1 = 1;
	else
	    input_wapp1 = 0;
	}

    inDataBuf = bpoint;
    varlen = XMLParser("<WAPP2>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    input_wapp2 = 1;
	else
	    input_wapp2 = 0;
	}
    inDataBuf = bpoint;
    varlen = XMLParser("<WAPP3>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    input_wapp3 = 1;
	else
	    input_wapp3 = 0;
	}
    inDataBuf = bpoint;
    varlen = XMLParser("<WAPP4>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    input_wapp4 = 1;
	else
	    input_wapp4 = 0;
	}
    inDataBuf = bpoint;
    varlen = XMLParser("<WAPP5>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    input_wapp5 = 1;
	else
	    input_wapp5 = 0;
	}
    inDataBuf = bpoint;
    varlen = XMLParser("<SMS1>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    input_sms1 = 1;
	else
	    input_sms1 = 0;
	}
    varlen = XMLParser("<SMS2>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    input_sms2 = 1;
	else
	    input_sms2 = 0;
	}
    varlen = XMLParser("<SMS3>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    input_sms3 = 1;
	else
	    input_sms3 = 0;
	}
    varlen = XMLParser("<SMS4>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    input_sms4 = 1;
	else
	    input_sms4 = 0;
	}
    varlen = XMLParser("<SMS5>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    input_sms5 = 1;
	else
	    input_sms5 = 0;
	}
    rBuf[0] = input_wapp1;
    rBuf[1] = input_wapp2;
    rBuf[2] = input_wapp3;
    rBuf[3] = input_wapp4;
    rBuf[4] = input_wapp5;

    EEwrite(EE_INPUT_WAPP1, rBuf, 5);

    rBuf[0] = input_sms1;
    rBuf[1] = input_sms2;
    rBuf[2] = input_sms3;
    rBuf[3] = input_sms4;
    rBuf[4] = input_sms5;
    EEwrite(EE_INPUT_SMS1, rBuf, 5);

    varlen = XMLParser("<DELAY1>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)	//--------ALARM DELAY
	{
	if (IntToFile(pb, varlen))
	    {
	    EEwrite_16(EE_INPUT_DELAY1, (pb[0] * 256 + pb[1]));
	    dinput[0].DELAY = pb[0] * 256 + pb[1];
	    }
	}
    varlen = XMLParser("<DELAY2>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)	//--------ALARM DELAY
	{
	if (IntToFile(pb, varlen))
	    {
	    EEwrite_16(EE_INPUT_DELAY2, (pb[0] * 256 + pb[1]));
	    dinput[1].DELAY = pb[0] * 256 + pb[1];
	    }
	}
    varlen = XMLParser("<DELAY3>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)	//--------ALARM DELAY
	{
	if (IntToFile(pb, varlen))
	    {
	    EEwrite_16(EE_INPUT_DELAY3, (pb[0] * 256 + pb[1]));
	    dinput[1].DELAY = pb[0] * 256 + pb[1];
	    }
	}
    return 1;
    }/***** SaveInput() *****/

/****************************************************************
 *****************************************************************/
UINT8 SaveMQTT(UINT8 *bpoint, UINT16 len)
    {
    UINT8 varlen, cnt, tmp8;

    inDataBuf = bpoint;

    varlen = XMLParser("<ENABLE>", (char*) inDataBuf, len, tb);
    if (varlen)
	{
	if (!strcmp(tb, "ON"))
	    tmp8 = 1;
	else
	    tmp8 = 0;
	EEwrite_8(EE_MQTT_ENABLE, tmp8);
	}

    varlen = XMLParser("<SERVER>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	StringToFile(rBuf, varlen);
    EEwrite(EE_MQTT_SERVER, rBuf, strlen(rBuf) + 1);

    varlen = XMLParser("<PORT>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	if (IntToFile(pb, varlen))
	    EEwrite(EE_MQTT_PORT, pb, 2);
	}

    varlen = XMLParser("<USERNAME>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	StringToFile(rBuf, varlen);
    EEwrite(EE_MQTT_UNAME, rBuf, strlen(rBuf) + 1);

    varlen = XMLParser("<PASS>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	StringToFile(rBuf, varlen);
    EEwrite(EE_MQTT_PASS, rBuf, strlen(rBuf) + 1);

    varlen = XMLParser("<TOPIC>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	StringToFile(rBuf, varlen);
    EEwrite(EE_MQTT_TOPIC, rBuf, strlen(rBuf) + 1);

    varlen = XMLParser("<KEEPALIVE>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	tmp8 = getTemp(varlen);
	EEwrite_8(EE_MQTT_KEEPALIVE, tmp8);
	}

    varlen = XMLParser("<QOS>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	tmp8 = getTemp(varlen);
	EEwrite_8(EE_MQTT_QOS, tmp8);
	}

    varlen = XMLParser("<PERIOD>", (char*) inDataBuf, len, tb);
    iBuf = tb;
    if (varlen)
	{
	tmp8 = getTemp(varlen);
	EEwrite_8(EE_MQTT_PERIOD, tmp8);
	}

    return 1;
    }/***** SaveMQTT() ****/

/****************************************************************
 *****************************************************************/
UINT8 JumpToBoot(UINT8 *bpoint, UINT16 len)
    {
    UINT8 initbyte;
    RTC_WriteBackupRegister(0, 0x55);//komanda da bootloader ostane u boot modu
    initbyte = RTC_ReadBackupRegister(0);
    while (1)
	{

	}
//    if (((*(__IO uint32_t*)BOOT_FLASH_FIRST_PAGE_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
//        {
//        __disable_irq ();
//        RTC_WriteBackupRegister(0,0x55);//komanda da bootloader ostane u boot modu
//
//        JumpAddress = *(__IO uint32_t*) (BOOT_FLASH_FIRST_PAGE_ADDRESS + 4);
//        Jump_To_Application = (pFunction) JumpAddress;
//        /* Initialize user application's Stack Pointer */
//        __set_MSP(*(__IO uint32_t*) BOOT_FLASH_FIRST_PAGE_ADDRESS);
//        Jump_To_Application();
//        }

    return 1;
    }/***** JumpToBoot() *****/
