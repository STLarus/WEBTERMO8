#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <W25Q256.h>
#include "strukture.h"
#include "define.h"
#include "drajveri.h"
#include "system.h"
#include "1wire.h"
#include "sys_ini.h"
#include "lwipopts.h"
#include "http_client.h"
#include "stm32f4xx_conf.h"
#include "lfs.h"

extern void my_smtp_test(void);

lfs_file_t xmlfile;

char tmpBuf[80];
unsigned char *bufstart;	//pointer na pocetak baffera
unsigned int xLen, bLen;
extern unsigned char gMonth, gDate, gHour, gMin, gDay, gSec;
extern char wcl_credit_active;
extern struct MAC_ADDR
    {
	uint8_t v[6];
    } mac_addr;
extern char readTemp[];
char XMLBuf[5 * TCP_SND_BUF] __attribute__ ((section(".ccmram")));
char lstr[50];
const char *sName[] =
    {
    "NAME1", "NAME2", "NAME3", "NAME4", "NAME5", "NAME6", "NAME7", "NAME8",
    };
const char *sTemp[] =
    {
    "TEMP1", "TEMP2", "TEMP3", "TEMP4", "TEMP5", "TEMP6", "TEMP7", "TEMP8",
    };
const char *sStat[] =
    {
    "STAT1", "STAT2", "STAT3", "STAT4", "STAT5", "STAT6", "STAT7", "STAT8",
    };
const char *sLimit[] =
    {
	    "LIMIT1",
	    "LIMIT2",
	    "LIMIT3",
	    "LIMIT4",
	    "LIMIT5",
	    "LIMIT6",
	    "LIMIT7",
	    "LIMIT8",
    };
const char *sSensor[] =
    {
	    "SENSOR1",
	    "SENSOR2",
	    "SENSOR3",
	    "SENSOR4",
	    "SENSOR5",
	    "SENSOR6",
	    "SENSOR7",
	    "SENSOR8"
    };
const char *sAlarm[] =
    {
	    "ALARM1",
	    "ALARM2",
	    "ALARM3",
	    "ALARM4",
	    "ALARM5",
	    "ALARM6",
	    "ALARM7",
	    "ALARM8"
    };
const char *sTLow[] =
    {
    "T1LOW", "T2LOW", "T3LOW", "T4LOW", "T5LOW", "T6LOW", "T7LOW", "T8LOW"
    };
const char *sTHigh[] =
    {
	    "T1HIGH",
	    "T2HIGH",
	    "T3HIGH",
	    "T4HIGH",
	    "T5HIGH",
	    "T6HIGH",
	    "T7HIGH",
	    "T8HIGH"
    };
const char *sDelay[] =
    {
	    "DELAY1",
	    "DELAY2",
	    "DELAY3",
	    "DELAY4",
	    "DELAY5",
	    "DELAY6",
	    "DELAY7",
	    "DELAY8"
    };

uint32_t totsect, freesect;

struct lfs_info tmpinfo;
lfs_dir_t tmpdir;

/**
 *
 * @param p
 * @param block
 * @return
 */
static int _traverse_df_cb(void *p, lfs_block_t block)
    {
    uint32_t *nb = p;
    *nb += 1;
    return 0;
    }

static uint32_t _diskfree(void)
    {
    int err;

    uint32_t _df_nballocatedblock = 0;
    //err = lfs_traverse(&wtlfs, _traverse_df_cb, &_df_nballocatedblock);
    if (err < 0)
	{
	return err;
	}

    uint32_t available = (LFS_BLOCKS_NUM - _df_nballocatedblock)
	    * LFS_BLOCK_SIZE;

    return available;
    }

/** \brief Pretvara vrijeme zapisa datoteke u ASCII niz
 * 	\author
 *	\li Teo Vuckovic (teo@larus.com.hr)
 *	\date 23.04.2019
 *	\param ftime	struktura tipa filetime
 *	\param fstr 	Pointer na niz u koji se sprema ASCII vrijeme
 *
 *
 *	Funkcija pretvara zapis o vremenu u ASCII niz slijede�eg formata
 *	HH:MM:SS
 */
void filetimetostr(struct _filetime *ftime, char *fstr)
    {
    UINT8 ch;
    char tbf[8];
    sprintf((char*) fstr, "%d", ftime->hour);	//sat
    strcat(fstr, ":");
    sprintf((char*) tbf, "%d", ftime->min);	//minuta
    strcat(fstr, tbf);
    strcat(fstr, ":");
    sprintf((char*) tbf, "%d", ftime->sec);	//sekunde
    strcat(fstr, tbf);
    }/***** filetimetostr() *****/

/** \brief Pretvara datum zapisa datoteke u ASCII niz
 * 	\author
 *	\li Teo Vuckovic (teo@larus.com.hr)
 *	\date 23.04.2019
 *	\param ftime	struktura tipa filetime
 *	\param fstr 	Pointer na niz u koji se sprema ASCII d
 *
 *
 *	Funkcija pretvara zapis o datumu u ASCII niz slijede�eg formata
 *	DD.MM.YYYY
 */
void filedatetostr(struct _filetime *ftime, char *fstr)
    {
    UINT8 ch;
    char tbf[8];
    sprintf((char*) fstr, "%d", ftime->date);
    strcat(fstr, ".");
    sprintf((char*) tbf, "%d", ftime->month);
    strcat(fstr, tbf);
    strcat(fstr, ".");
    sprintf((char*) tbf, "%d", ftime->year);
    strcat(fstr, tbf);
    }/***** filedatetostr() *****/

/** \brief Pretvara vrijeme zapisa datoteke u ASCII niz
 * 	\author
 *		\li Teo Vuckovic (teo@larus.com.hr)
 *	\date 01.10.2010
 *	\param ftime	Vrijeme u 16-bitnom formatu
 *	\param fstr 	Pointer na niz u koji se sprema ASCII vrijeme
 *
 *	Funkcija uzima zapis o vremenu u formatu:
 *	HHHHHMMM MMMSSSSS
 *	i pretvara ga u ASCII niz slijedeæeg formata
 *	HH:MM:SS
 */
void ftimetostr(UINT16 ftime, char *fstr)
    {
    UINT8 ch;
    char tbf[8];
    ch = (UINT8) (ftime >> 11);
    sprintf((char*) fstr, "%d", ch);	//sat
    strcat(fstr, ":");
    ch = (UINT8) ((ftime & 0x07E0) >> 5);
    sprintf((char*) tbf, "%d", ch);	//minuta
    strcat(fstr, tbf);
    strcat(fstr, ":");
    ch = (UINT8) ((ftime & 0x001F) << 1);
    sprintf((char*) tbf, "%d", ch);	//sekunde
    strcat(fstr, tbf);
    }/***** ftimetostr() *****/

/** \brief Pretvara datum zapisa datoteke u ASCII niz
 * 	\author
 *		\li Teo Vuckovic (teo@larus.com.hr)
 *	\date 01.10.2010
 *	\param fdate	Datum u 16-bitnom formatu
 *	\param fstr 	Pointer na niz u koji se sprema ASCII vrijeme
 *
 *	Funkcija uzima zapis o datumu u formatu:
 *	YYYYYYYM MMMDDDDD
 *	i pretvara ga u ASCII niz slijedeæeg formata
 *	DD.MM.YYYY
 *	Godina je u formatu 1980+YYYYY
 */
void fdatetostr(UINT16 fdate, char *fstr)
    {
    UINT16 yr;
    char tbf[8];
    yr = (fdate & 0x001F);
    sprintf((char*) fstr, "%d", yr);	//datum
    strcat(fstr, ".");
    yr = ((fdate & 0x01E0) >> 5);
    sprintf((char*) tbf, "%d", yr);	//mjesec
    strcat(fstr, tbf);
    strcat(fstr, ".");
    yr = (fdate >> 9);
    sprintf((char*) tbf, "%d", (yr + 1980));	//godina
    strcat(fstr, tbf);
    }/***** fdatetostr() *****/

/** \brief ispituje da li je extenzija fajla csv
 * 	\author
 *		\li Teo Vuckovic (teo@larus.com.hr)
 *	\date 12.08.2011
 *	\param *fname	pointer na ime gajla u formatu 8.3
 *
 *	Funkcija uzima ime fajla u formatu 8.3 te ispituje da li je
 *	extenzija tipa csv. Ako je vraæa TRUE ako nije vraæa FALSE
 */
char isfextcsv(char *fname)
    {
    char cnt;
    char ext[5];
    for (cnt = 0; cnt < 9; cnt++)
	{
	if (*fname++ == '.')
	    break;
	}
    cnt = 0;
    while ((*(fname) != 0))
	{
	ext[cnt++] = tolower(*(fname++));
	}
    ext[cnt] = 0;

    if (strcmp(ext, "csv") == 0)
	return TRUE;

    else
	return FALSE;
    }/***** isfextcsv() *****/

/** \brief kreira AJAX string sa gornjom i donjom granicom temperature
 * 	\author
 *	\li Teo Vuckovic (teo@larus.com.hr)
 *	\date 27.10.2010
 *	\param lbuf	pointer na bufffer u koji se upisuje string
 *	\param llow	donja granièna temperatura
 *	\param lhigh gornjaa granièna temperatura
 *
 *	Funkcija gornju i donju granicu alarma pretvara u AJAX string. Npr. ako je
 *  donja granica 10 C a gornja 13C string je oblika "10 - 13"
 */
void create_limit_string(char *lbuf, INT8 llow, INT8 lhigh)
    {
    sprintf(lbuf, "%d", llow);
    strcat(lbuf, " ..... ");
    lbuf += strlen(lbuf);
    sprintf(lbuf, "%d", lhigh);
    }

/*------------------------------------------------------------------------------**
 ** Funkcija long varijablu pretvara u string oblika IP adrese					**
 **																				**
 ** *buf.......pointer na array u koji se spremaju podaci						**
 ** lAdr.......IP adresa u long formatu											**
 ** return.... broj upisanih podataka u bufferu podaci							**
 **------------------------------------------------------------------------------*/
UINT8 LONGtoIP(char *buf, UINT32 lAdr)
    {
    unsigned char cnt, slen, tlen = 0;
    unsigned char *pr, *tr;
    unsigned long tAdr;
//*** zbog formata zapisa potrebno je sve bajtove zarotirati
    tAdr = lAdr;
    tr = (unsigned char*) &tAdr;
    pr = (unsigned char*) &lAdr;
    *pr = *(tr + 3);
    *(pr + 1) = *(tr + 2);
    *(pr + 2) = *(tr + 1);
    *(pr + 3) = *(tr + 0);	////zarotirano
    for (cnt = 0; cnt < 4; cnt++)
	{
	slen = sprintf((char*) buf, "%d", *pr);
	buf += slen;
	tlen += slen;
	*(buf++) = '.';
	tlen++;
	pr++;
	}
    tlen--;	//brise cetvrtu tocku
    buf--;
    *buf = 0;	//kraj stringa
    return tlen;
    }/***** LONGtoIP() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira XML header                                            **
 **  	HTTP/1.0 200 OK\r\n												 **
 ** 		Server: LARUS-2561/1.0\nContent-type: application/xml\r\n		 **
 **      Content-length: 111\r\n\r\n                                      **
 **                                                                       **
 ** return.... broj upisanih podataka u bufferu podaci                    **
 **-----------------------------------------------------------------------*/
unsigned char CreateXMLHeader(void)
    {
    strcpy(XMLBuf, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>");
    return strlen(XMLBuf);
    }/***** CreateXMLHeader() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija otvara XML tag i upisuje ga u XMLbuffer te podesava pointere **
 **                                                                       **
 ** *pTAg......pointer na string koji kreira xml tag                      **
 ** *bufVal....pointer na string koji nosi vrijednost XML taga           **
 ** return.... broj upisanih podataka u bufferu podaci                    **
 **-----------------------------------------------------------------------*/
unsigned int AddTag(const char *pTag, char *bufVal)
    {
    strcat(XMLBuf, "<");
    strcat(XMLBuf, pTag);
    strcat(XMLBuf, ">");
    strcat(XMLBuf, bufVal);
    strcat(XMLBuf, "</");
    strcat(XMLBuf, pTag);
    strcat(XMLBuf, ">");
    return 1;
    }/***** AddTag() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija otvara XML tag i pretvara karakter u ASCII                   **
 **                                                                       **
 ** *pTAg......pointer na string koji kreira xml tag                      **
 ** xval.......numerièki podatak koji pretvara u ASCII					 **
 ** return.... broj upisanih podataka u bufferu podaci                    **
 **-----------------------------------------------------------------------*/
unsigned int AddNumTag(const char *pTag, UINT16 xval)
    {
    char tBuf[10];
    sprintf(tBuf, "%d", xval);
    strcat(XMLBuf, "<");
    strcat(XMLBuf, pTag);
    strcat(XMLBuf, ">");
    strcat(XMLBuf, tBuf);
    strcat(XMLBuf, "</");
    strcat(XMLBuf, pTag);
    strcat(XMLBuf, ">");
    return 1;
    }/***** AddNumTag() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija otvara XML tag i pretvara temperaturu u ASCII                **
 Temperature su u opsegu -55 do 130
 ** *pTAg......pointer na string koji kreira xml tag                      **
 ** xval.......numerièki podatak koji pretvara u ASCII					 **
 ** return.... broj upisanih podataka u bufferu podaci                    **
 **-----------------------------------------------------------------------*/
unsigned int AddTempTag(const char *pTag, INT8 xval)
    {
    char tBuf[10];
    if (xval < 0)
	{
	//negativna temperatura
	xval = 255 - xval + 1;
	tBuf[0] = '-';
	sprintf(&tBuf[1], "%d", xval);
	}
    else
	sprintf(tBuf, "%d", xval);	//pozitivna temperatura
    strcat(XMLBuf, "<");
    strcat(XMLBuf, pTag);
    strcat(XMLBuf, ">");
    strcat(XMLBuf, tBuf);
    strcat(XMLBuf, "</");
    strcat(XMLBuf, pTag);
    strcat(XMLBuf, ">");
    return 1;
    }/***** AddNumTag() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija otvara XML tag i upisuje ga u XMLbuffer te podesava pointere **
 **                                                                       **
 ** *pTAg......pointer na string koji kreira xml tag                      **
 ** return.... broj upisanih podataka u bufferu podaci                    **
 **-----------------------------------------------------------------------*/
unsigned int OpenTag(char const *pTag)
    {
    strcat(XMLBuf, "<");
    strcat(XMLBuf, pTag);
    strcat(XMLBuf, ">");
    return 1;
    }/***** OpenTag() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija zatvara XML tag i upisuje ga u XMLbuffer                     **
 **                                                                       **
 ** *pTAg......pointer na string koji kreira xml tag                      **
 ** return.... broj upisanih podataka u bufferu podaci                    **
 **-----------------------------------------------------------------------*/
unsigned int CloseTag(char const *pTag)
    {
    strcat(XMLBuf, "</");
    strcat(XMLBuf, pTag);
    strcat(XMLBuf, ">");
    return 1;
    }/***** CloseTag() *****/

void addLength(unsigned int ctLen)
    {
    unsigned char a;
    bufstart = (unsigned char*) &XMLBuf[0];
    do
	{
	a = *(bufstart++);
	}
    while (a != '#');
    bufstart--;
    a = sprintf((char*) (bufstart), "%d", ctLen);
    if (a)
	*(bufstart + a) = 0x20;	//briše zadnju nulu koju je ubacila funkcija sprintf
    }/***** addLength() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira sms.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLSms(UINT8 *xmlBuf)
    {
    UINT32 Adresa;
    *infobip_credit = 0;

    CreateXMLHeader();
    OpenTag("SMS");
//    READ_FLASH(0xC00,rBuf,256);
    Adresa = EEread_32(EE_GSM_IP);
    LONGtoIP(tmpBuf, Adresa);
    AddTag("IP", tmpBuf);	//--------<IP>
    Adresa = EEread_16(EE_GSM_PORT);
    AddNumTag("PORT", Adresa);

    EEread(EE_GSM_UNAME, tbuf, 16);
    AddTag("UNAME", tbuf);
    EEread(EE_GSM_PASS, tbuf, 16);
    AddTag("PASS", tbuf);
    EEread(EE_GSM1_NUM, tbuf, 32);
    AddTag("GSM1", tbuf);
    EEread(EE_GSM2_NUM, tbuf, 32);
    AddTag("GSM2", tbuf);
    EEread(EE_GSM3_NUM, tbuf, 32);
    AddTag("GSM3", tbuf);
    EEread(EE_GSM4_NUM, tbuf, 32);
    AddTag("GSM4", tbuf);
    EEread(EE_GSM5_NUM, tbuf, 32);
    AddTag("GSM5", tbuf);

    EEread(EE_GSM1_NAME, tbuf, 32);
    AddTag("NAME1", tbuf);
    EEread(EE_GSM2_NAME, tbuf, 32);
    AddTag("NAME2", tbuf);
    EEread(EE_GSM3_NAME, tbuf, 32);
    AddTag("NAME3", tbuf);
    EEread(EE_GSM4_NAME, tbuf, 32);
    AddTag("NAME4", tbuf);
    EEread(EE_GSM5_NAME, tbuf, 32);
    AddTag("NAME5", tbuf);

    CloseTag("SMS");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLSms() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira Mreza.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLMreza(UINT8 *xmlBuf)
    {
    uint8_t tmp;
    UINT8 cnt;
    INT8 *tbuf, slen;
    UINT32 Adresa;
    CreateXMLHeader();
    OpenTag("IPMODULA");
    tmp = EEread_8(EE_DHCP_STATE);	//--------<DHCP>
    /*    if (tmp == 0)
     AddTag("DHCP", "OFF");
     else
     AddTag("DHCP", "ON");
     tmp = EEread_8(EE_DDNS_STATE);	//--------<DDNS>
     if (tmp == 0)
     AddTag("DDNS", "OFF");
     else
     AddTag("DDNS", "ON");
     */
    Adresa = EEread_32(EE_IP);
    LONGtoIP(tmpBuf, Adresa);
    AddTag("IP", tmpBuf);	//--------<IP>
    tbuf = (INT8*) &tmpBuf[0];
    for (cnt = 0; cnt < 6; cnt++)
	{
	tmp = mac_addr.v[cnt];
	slen = sprintf((char*) tbuf, "%X", tmp);
	tbuf += slen;
	*(tbuf++) = ':';
	}
    *(--tbuf) = 0;
    AddTag("MAC", tmpBuf);	//--------<MAC>
    Adresa = EEread_32(EE_SUBNET);
    LONGtoIP(tmpBuf, Adresa);
    AddTag("SUB", tmpBuf);	//--------<SUB>
    Adresa = EEread_32(EE_GATEWAY);
    LONGtoIP(tmpBuf, Adresa);
    AddTag("ROUT", tmpBuf);	//--------<ROUT>
    Adresa = EEread_32(EE_DNS);
    LONGtoIP(tmpBuf, Adresa);
    AddTag("DNS", tmpBuf);	//--------<DNS>
    Adresa = EEread_16(EE_HTTP_PORT);
    sprintf(tmpBuf, "%d", Adresa);
    AddTag("HTTP", tmpBuf);	//--------<HTTP>
    /*    EEread(EE_USERNAME, twi_buf, 16);
     AddTag("USER", twi_buf);	//--------<USER>
     EEread(EE_PASS, twi_buf, 16);
     AddTag("PASS", twi_buf);	//--------<PASS>

     EEread(EE_DDNS_HOST, twi_buf, 32);
     AddTag("HOST", twi_buf);	//--------<HOST>
     */
    EEread(EE_OBJECT_NAME, twi_buf, 32);
    AddTag("NAZIV", twi_buf);	//--------<NAZIV>
    Adresa = EEread_32(EE_ALARM_IP);
    LONGtoIP(tmpBuf, Adresa);
    AddTag("ALARMIP", tmpBuf);	//--------<ALARMIP>
    Adresa = EEread_16(EE_ALARM_PORT);
    sprintf(tmpBuf, "%d", Adresa);
    AddTag("ALARMPORT", tmpBuf);	//--------<ALARMPORT>

    CloseTag("IPMODULA");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLMreza() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira ulazi.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLUlazi(UINT8 *xmlBuf)
    {
    UINT16 tmp;

    CreateXMLHeader();
    OpenTag("ULAZI");

    if (IN1_STATE != 0)
	AddTag("IN1", "CLOSE");
    else
	AddTag("IN1", "OPEN");
    if (IN2_STATE != 0)
	AddTag("IN2", "CLOSE");
    else
	AddTag("IN2", "OPEN");
    if (IN3_STATE != 0)
	AddTag("IN3", "CLOSE");
    else
	AddTag("IN3", "OPEN");
    CloseTag("ULAZI");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);

    }/***** XMLUlazi() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira izlazi.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLIzlazi(UINT8 *xmlBuf)
    {

    return strlen(XMLBuf);
    }/***** XMLIzlazi() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira statusi.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLStatusi(UINT8 *xmlBuf)
    {

    return strlen(XMLBuf);
    }/***** XMLStatusi() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira Sustav.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLSustav(UINT8 *xmlBuf)
    {
    UINT8 t, strl;
    CreateXMLHeader();
    OpenTag("SUSTAV");
//AddTag("NAME",(char *)net_ini.DEVICE_NAME);
//AddTag("VER",VERZIJA);
//t=0;
//strl=sprintf((char*)&tmpBuf[t],"%d",gDate);
//t+=strl;
//tmpBuf[t++]='.';
//strl=sprintf((char*)&tmpBuf[t],"%d",gMonth);
//t+=strl;
//tmpBuf[t++]='.';
//strl=sprintf((char*)&tmpBuf[t],"%d",gYear);
//t+=strl;
//tmpBuf[t++]=' ';
//tmpBuf[t++]=' ';
//strl=sprintf((char*)&tmpBuf[t],"%d",gHour);
//t+=strl;
//tmpBuf[t++]=':';
//strl=sprintf((char*)&tmpBuf[t],"%d",gMin);
//t+=strl;
//tmpBuf[t++]=':';
//strl=sprintf((char*)&tmpBuf[t],"%d",gSec);
//t+=strl;
//tmpBuf[t]=0;
//AddTag("TIME",tmpBuf);
//if(pac_ini.GRAPH_ENABLE)
//	AddTag("GRAPH","ON");
//else
//	AddTag("GRAPH","OFF");
//LONGtoIP(tmpBuf,pac_ini.IP);AddTag("PACIP",tmpBuf);
//sprintf((char*)tmpBuf,"%d",pac_ini.FEED);AddTag("FEED",tmpBuf);
////AddTag("APIKEY",(char *)pac_ini.APIKEY);
//sprintf((char*)tmpBuf,"%d",pac_ini.SEC);AddTag("SEC",tmpBuf);
//CloseTag("SUSTAV");
//xLen=strlen(XMLBuf);
//strcpy((char*)xmlBuf,XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLSustav() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira Time.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLTime(UINT8 *xmlBuf)
    {
    CreateXMLHeader();
    getTime(&sat, &minuta, &sekunda);
    getDate(&godina, &mjesec, &datum, &dan);
    OpenTag("TIME");
    sprintf((char*) tmpBuf, "%d", sat);
    AddTag("HOUR", tmpBuf);
    sprintf((char*) tmpBuf, "%d", minuta);
    AddTag("MIN", tmpBuf);
    sprintf((char*) tmpBuf, "%d", sekunda);
    AddTag("SEC", tmpBuf);

    sprintf((char*) tmpBuf, "%d", datum);
    AddTag("DATE", tmpBuf);
    sprintf((char*) tmpBuf, "%d", mjesec);
    AddTag("MONTH", tmpBuf);
    sprintf((char*) tmpBuf, "%d", godina + 2000);
    AddTag("YEAR", tmpBuf);
    sprintf((char*) tmpBuf, "%d", dan);
    AddTag("DAY", tmpBuf);
    CloseTag("TIME");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLTime() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira XML tag sa izlistom svih fajlova na disku             **
 **                                                                       **
 ** *xmlBuf....pointer na string koji kreira xml tag                      **
 ** disk.......broj diska 0 ili 1										 **
 ** return.... broj upisanih podataka u bufferu                           **
 **-----------------------------------------------------------------------*/
UINT16 XMLDisk(UINT8 *xmlBuf, UINT8 disk)
    {
    uint16_t fdate, ftime;
    int err;
    uint32 fattime;
    uint16_t year;
    uint8_t month, date, hour, min, sec;
    uint32_t u32tmp;

    lfs_dir_t wtdir;
    struct lfs_info wtinfo;

    CreateXMLHeader();
    OpenTag("DISK0");

    lfs_ssize_t used_blocks = lfs_fs_size(&wtlfs);
    totsect = wtcfg.block_count * LFS_BLOCK_SIZE;
    freesect = totsect - used_blocks * LFS_BLOCK_SIZE;

    OpenTag("DINFO");
    sprintf((char*) &lstr, "%ld", freesect / 1024);
    AddTag("FREE", (char*) lstr);
    sprintf((char*) &lstr, "%ld", totsect / 1024);
    AddTag("SIZE", (char*) lstr);
    CloseTag("DINFO");

    lfs_dir_open(&wtlfs, &wtdir, "");
    lfs_dir_rewind(&wtlfs, &wtdir);
    while (true)
	{
	int res = lfs_dir_read(&wtlfs, &wtdir, &wtinfo);
	if (res < 0)
	    return res;

	if (res == 0)
	    break;

	if (wtinfo.type == LFS_TYPE_REG)
	    {
	    lfs_getattr(&wtlfs, wtinfo.name, ATTR_TIMESTAMP, &filetime,
		    sizeof(filetime));

	    OpenTag("LF");
	    AddTag("NM", (char*) wtinfo.name);
	    sprintf((char*) &lstr, "%ld", wtinfo.size);
	    AddTag("SZ", lstr);
	    u32tmp = (uint16_t) (fattime >> 16);
	    filedatetostr(&filetime, lstr);
	    AddTag("DT", lstr);
	    u32tmp = (uint16_t) (fattime & 0x0000FFFF);
	    filetimetostr(&filetime, lstr);
	    AddTag("TM", lstr);
	    CloseTag("LF");
	    }
	}
    err = lfs_dir_close(&wtlfs, &wtdir);
    CloseTag("DISK0");
    return strlen(XMLBuf);
    }/***** XMLDisk() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira izlist podataka sa diska 0                            **
 **-----------------------------------------------------------------------*/
UINT16 XMLDisk0(UINT8 *xmlBuf)
    {
    UINT16 retval;
    retval = XMLDisk(xmlBuf, 0);
    return retval;
    }/***** XMLDisk0() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira izlist podataka sa diska 1                            **
 **-----------------------------------------------------------------------*/
UINT16 XMLDisk1(UINT8 *xmlBuf)
    {
    UINT16 retval;
    retval = XMLDisk(xmlBuf, 1);
    f_chdrive(0);
    return retval;

    }/***** XMLDisk1() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira XML tag sa izlistom svih datalog fajlova na disku 1   **
 **                                                                       **
 ** *xmlBuf....pointer na string koji kreira xml tag                      **
 ** return.... broj upisanih podataka u bufferu                           **
 **-----------------------------------------------------------------------*/
UINT16 XMLLogfile(UINT8 *xmlBuf, UINT8 disk)
    {

    struct lfs_dir wtdir;
    struct lfs_info wtinfo;
    CreateXMLHeader();

    OpenTag("LOGFILE");
    OpenTag("SENZOR");
    EEread(0x100 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 1
    AddTag("SEN1", (char*) tbuf);
    AddTempTag("LOW1", senzor[0].LOWLIMIT);
    AddTempTag("HIGH1", senzor[0].HIGHLIMIT);

    EEread(0x200 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 2
    AddTag("SEN2", (char*) tbuf);
    AddTempTag("LOW2", senzor[1].LOWLIMIT);
    AddTempTag("HIGH2", senzor[1].HIGHLIMIT);

    EEread(0x300 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 3
    AddTag("SEN3", (char*) tbuf);
    AddTempTag("LOW3", senzor[2].LOWLIMIT);
    AddTempTag("HIGH3", senzor[2].HIGHLIMIT);

    EEread(0x400 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 4
    AddTag("SEN4", (char*) tbuf);
    AddTempTag("LOW4", senzor[3].LOWLIMIT);
    AddTempTag("HIGH4", senzor[3].HIGHLIMIT);

    EEread(0x500 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 5
    AddTag("SEN5", (char*) tbuf);
    AddTempTag("LOW5", senzor[4].LOWLIMIT);
    AddTempTag("HIGH5", senzor[4].HIGHLIMIT);

    EEread(0x600 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 6
    AddTag("SEN6", (char*) tbuf);
    AddTempTag("LOW6", senzor[5].LOWLIMIT);
    AddTempTag("HIGH6", senzor[5].HIGHLIMIT);

    EEread(0x700 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 7
    AddTag("SEN7", (char*) tbuf);
    AddTempTag("LOW7", senzor[6].LOWLIMIT);
    AddTempTag("HIGH7", senzor[6].HIGHLIMIT);

    EEread(0x800 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 8
    AddTag("SEN8", (char*) tbuf);
    AddTempTag("LOW8", senzor[7].LOWLIMIT);
    AddTempTag("HIGH8", senzor[7].HIGHLIMIT);

//    int err = lfs_mount(&wtlfs, &wtcfg);

    CloseTag("SENZOR");

    lfs_dir_open(&wtlfs, &wtdir, "");
    lfs_ssize_t used_blocks = lfs_fs_size(&wtlfs);
    totsect = wtcfg.block_count * LFS_BLOCK_SIZE;
    freesect = totsect - used_blocks * LFS_BLOCK_SIZE;

    OpenTag("DINFO");
    sprintf((char*) &lstr, "%ld", freesect / 1024);
    AddTag("FREE", (char*) lstr);
    sprintf((char*) &lstr, "%ld", totsect / 1024);
    AddTag("SIZE", (char*) lstr);
    CloseTag("DINFO");

    while (true)
	{
	int res = lfs_dir_read(&wtlfs, &wtdir, &wtinfo);
	if (res < 0)
	    {
	    return res;
	    }

	if (res == 0)
	    {
	    break;
	    }

	if (wtinfo.type == LFS_TYPE_REG)
	    {
	    if (wtinfo.name[0] == 'T')
		{
		OpenTag("LF");
		AddTag("LOG", wtinfo.name);
		CloseTag("LF");
		}
	    }
	}
    lfs_dir_close(&wtlfs, &wtdir);
    CloseTag("LOGFILE");

    /*    strcpy(tmpBuf, DISKA);
     strcat(tmpBuf, "/");
     dir_info = ionFS_opendir(_TC(tmpBuf));

     if(dir_info != NULL)
     {
     ionFS_set_errno( IONFS_OK );
     dir_ent = ionFS_readdir( dir_info );
     while ( NULL != dir_ent )
     {
     dir_ent = ionFS_readdir(dir_info);
     if(strlen(dir_ent->d_name))
     {
     if((*dir_ent->d_name)=='T')
     {
     OpenTag("LF");
     AddTag("LOG",dir_ent->d_name);
     CloseTag("LF");
     }
     }

     }
     }
     ionFS_closedir(dir_info);
     CloseTag("LOGFILE");*/

    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);

    }/***** XMLogfile() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira XML tag sa izlistom svih alarmnih fajlova na disku 1  **
 **                                                                       **
 ** *xmlBuf....pointer na string koji kreira xml tag                      **
 ** return.... broj upisanih podataka u bufferu                           **
 **-----------------------------------------------------------------------*/
UINT16 XMLAlarmfile(UINT8 *xmlBuf, UINT8 disk)
    {
    CreateXMLHeader();
    OpenTag("LOGFILE");
//    f_chdrive(0);
//    fresult=f_opendir(&jdir,"");	//otvara direktorij za èitanje fajlova
//    while(1)
//    {
//        fresult=f_readdir(&jdir,&fno);
//        if(fresult!=FR_OK || fno.fname[0]==0)
//        {
//            CloseTag("LOGFILE");
//            f_chdrive(0);
//            strcpy((char*)xmlBuf,XMLBuf);
//            return strlen(XMLBuf);
//        }
//        else if(fno.fname[0]=='.')
//        {}
//        else
//        {
//            if(isfextcsv(fno.fname)==TRUE)
//            {
//                if((*fno.fname)=='S' && (*(fno.fname+1))=='E')
//                {
//                    OpenTag("SEN");
//                    AddTag("SZ",(char *)fno.fname);
//                    sprintf((char*)&lstr,"%ld",fno.fsize);
//                    CloseTag("SEN");
//                }
//                else if((*fno.fname)=='A')
//                {
//                    OpenTag("LF");
//                    AddTag("LOG",(char *)fno.fname);
//                    sprintf((char*)&lstr,"%ld",fno.fsize);
//                    CloseTag("LF");
//                }
//            }
//
//        }
//    }
    }/***** XMAlarmfile() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira XMLCode.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLCode(UINT8 *xmlBuf)
    {
    UINT8 cnt, bcode[10];
    UINT16 itmp;

    CreateXMLHeader();
    OpenTag("DSCODE");
    DS18B20Code(bcode);
    /*** treba provjeriti status da li je sve OK ***/
    for (cnt = 0; cnt < 30; cnt++)
	lstr[cnt] = 0;
    for (cnt = 0; cnt < 8; cnt++)
	{
	itmp = hextoascii(bcode[cnt]);
	lstr[cnt * 2] = (UINT8) (itmp >> 8);
	lstr[cnt * 2 + 1] = (UINT8) (itmp & 0x00FF);
	}
    AddTag("CODE", (char*) lstr);
    CloseTag("DSCODE");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLCode.xml() *****/

/*-------------------------------------------------------------------**
 ** Funkcija kreira Log.xml file										 **
 **-------------------------------------------------------------------*/
UINT16 XMLLog(UINT8 *xmlBuf)
    {
    return bLen;
    }/***** XMLLog() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira XMLSensor1.xml file									 **
 **-----------------------------------------------------------------------*/
UINT16 XMLSensor(UINT8 snum, UINT8 *xmlBuf)
    {
    uint8_t cnt;
    UINT16 itmp, tCalibCode;
    uint8_t sms1, sms2, sms3, sms4, sms5;
    uint8_t wapp1, wapp2, wapp3, wapp4, wapp5;
    CreateXMLHeader();
    OpenTag("SENSOR");
    AddTempTag("LOWLIMIT", senzor[snum].LOWLIMIT);
    AddTempTag("HIGHLIMIT", senzor[snum].HIGHLIMIT);
    AddTempTag("HIST", senzor[snum].HIS);
    my_ftoa(senzor[snum].kalibracija, lstr, 1);
    AddTag("KALIB", (char*) lstr);

    if (senzor[snum].ENBLE == 1)
	AddTag("ENABLE", "ON");
    else
	AddTag("ENABLE", "OFF");
    for (cnt = 0; cnt < 30; cnt++)
	lstr[cnt] = 0;
    for (cnt = 0; cnt < 8; cnt++)
	{
	itmp = hextoascii(senzor[snum].CODE[cnt]);
	lstr[cnt * 2] = (UINT8) (itmp >> 8);
	lstr[cnt * 2 + 1] = (UINT8) (itmp & 0x00FF);
	}
    AddTag("SERIALNUM", (char*) lstr);
    EEread(0x100 * (snum + 1) + EE_SENSOR_NAME, lstr, 32);
    AddTag("NAME", (char*) lstr);
    AddTempTag("KANAL", senzor[snum].KANAL);
    cnt = EEread_8(0x100 * (snum + 1) + EE_MEASURE_TYPE);
    if (cnt == 1)
	AddTag("MTYPE", "MKT");
    else
	AddTag("MTYPE", "AVERAGE");
    itmp = EEread_16(0x100 * (snum + 1) + EE_DELAY);
    AddNumTag("DELAY", itmp);

    EEread(0x100 * (snum + 1) + EE_SMS1, rBuf, 5);
    sms1 = rBuf[0];
    sms2 = rBuf[1];
    sms3 = rBuf[2];
    sms4 = rBuf[3];
    sms5 = rBuf[4];

    EEread(0x100 * (snum + 1) + EE_WAPP1_ON, rBuf, 5);
    wapp1 = rBuf[0];
    wapp2 = rBuf[1];
    wapp3 = rBuf[2];
    wapp4 = rBuf[3];
    wapp5 = rBuf[4];

    if (wapp1 == 0)
	AddTag("WAPP1", "OFF");
    else
	AddTag("WAPP1", "ON");
    if (wapp2 == 0)
	AddTag("WAPP2", "OFF");
    else
	AddTag("WAPP2", "ON");
    if (wapp3 == 0)
	AddTag("WAPP3", "OFF");
    else
	AddTag("WAPP3", "ON");
    if (wapp4 == 0)
	AddTag("WAPP4", "OFF");
    else
	AddTag("WAPP4", "ON");
    if (wapp5 == 0)
	AddTag("WAPP5", "OFF");
    else
	AddTag("WAPP5", "ON");
    if (sms1 == 0)
	AddTag("SMS1", "OFF");
    else
	AddTag("SMS1", "ON");
    if (sms2 == 0)
	AddTag("SMS2", "OFF");
    else
	AddTag("SMS2", "ON");
    if (sms3 == 0)
	AddTag("SMS3", "OFF");
    else
	AddTag("SMS3", "ON");
    if (sms4 == 0)
	AddTag("SMS4", "OFF");
    else
	AddTag("SMS4", "ON");
    if (sms5 == 0)
	AddTag("SMS5", "OFF");
    else
	AddTag("SMS5", "ON");

    CloseTag("SENSOR");
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLSensor.xml() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira XMLSensor1.xml file									 **
 **-----------------------------------------------------------------------*/
UINT16 XMLSensor1(UINT8 *xmlBuf)
    {
    UINT16 retval;
    retval = XMLSensor(0, xmlBuf);
    return retval;
    }/***** XMLSensor1.xml() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira XMLSensor2.xml file									 **
 **-----------------------------------------------------------------------*/
UINT16 XMLSensor2(UINT8 *xmlBuf)
    {
    UINT16 retval;
    retval = XMLSensor(1, xmlBuf);
    return retval;
    }/***** XMLSensor2.xml() *****/

UINT16 XMLSensor3(UINT8 *xmlBuf)
    {
    UINT16 retval;
    retval = XMLSensor(2, xmlBuf);
    return retval;
    }/***** XMLSensor3.xml() *****/

UINT16 XMLSensor4(UINT8 *xmlBuf)
    {
    UINT16 retval;
    retval = XMLSensor(3, xmlBuf);
    return retval;
    }/***** XMLSensor4.xml() *****/

UINT16 XMLSensor5(UINT8 *xmlBuf)
    {
    UINT16 retval;
    retval = XMLSensor(4, xmlBuf);
    return retval;
    }/***** XMLSensor5.xml() *****/

UINT16 XMLSensor6(UINT8 *xmlBuf)
    {
    UINT16 retval;
    retval = XMLSensor(5, xmlBuf);
    return retval;
    }/***** XMLSensor6.xml() *****/

UINT16 XMLSensor7(UINT8 *xmlBuf)
    {
    UINT16 retval;
    retval = XMLSensor(6, xmlBuf);
    return retval;
    }/***** XMLSensor7.xml() *****/

UINT16 XMLSensor8(UINT8 *xmlBuf)
    {
    UINT16 retval;
    retval = XMLSensor(7, xmlBuf);
    return retval;
    }/***** XMLSensor8.xml() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira XMLHome.xml file									 **
 **-----------------------------------------------------------------------*/
UINT16 XMLHome(UINT8 *xmlBuf)
    {
    char cnt;
    char tbf[8];
    DWORD freesect, totsect, freeclust;

    lfs_ssize_t used_blocks = lfs_fs_size(&wtlfs);
    totsect = wtcfg.block_count * LFS_BLOCK_SIZE;
    freesect = totsect - used_blocks * LFS_BLOCK_SIZE;

    getTime(&sat, &minuta, &sekunda);
    getDate(&godina, &mjesec, &datum, &dan);
    CreateXMLHeader();
    OpenTag("STATUSI");
    sprintf((char*) &lstr, "%ld", freesect / 1000);
    AddTag("FREE", (char*) lstr);
    sprintf((char*) &lstr, "%ld", totsect / 1000);
    AddTag("SIZE", (char*) lstr);
    EEread(EE_OBJECT_NAME, rBuf, 32);
    AddTag("NAME", rBuf);	//--------<NAZIV>

    sprintf((char*) lstr, "%d", datum);
    strcat(lstr, ".");
    sprintf((char*) tbf, "%d", mjesec);
    strcat(lstr, tbf);
    strcat(lstr, ".");
    sprintf((char*) tbf, "%d", godina + 2000);
    strcat(lstr, tbf);
    strcat(lstr, "  |  ");
    sprintf((char*) tbf, "%d", sat);
    strcat(lstr, tbf);
    strcat(lstr, ":");
    sprintf((char*) tbf, "%d", minuta);
    strcat(lstr, tbf);
    strcat(lstr, ":");
    sprintf((char*) tbf, "%d", sekunda);
    strcat(lstr, tbf);
    AddTag("DATUM", lstr);
    AddTag("VERZIJA", VERZIJA);
    for (cnt = 0; cnt < NUMBER_OF_SENSOR; cnt++)
	{
	EEread((0x100 * (cnt + 1) + EE_SENSOR_NAME), rBuf, 64);
	AddTag(sName[cnt], rBuf);	//--------<NAZIV>
	if (senzor[cnt].ENBLE) //***** SENSOR [i]
	    {
	    if (state[cnt] == OK)
		AddTag(sStat[cnt], "OK");
	    else
		AddTag(sStat[cnt], "ERROR");
	    }
	else
	    AddTag(sStat[cnt], "DISABLE");
	temp_to_ascii(temp[cnt], tmpBuf);
	AddTag(sTemp[cnt], tmpBuf);
	}
    CloseTag("STATUSI");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLHome() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira XMLOnline.xml file									 **
 **-----------------------------------------------------------------------*/
UINT16 XMLOnline(UINT8 *xmlBuf)
    {
    UINT8 cnt;
    char tbf[8];
    uint32_t delaytimer, delaysec;
    uint16_t delayhh, delaymm, delayss;

    CreateXMLHeader();
    OpenTag("ONLINE");

    getTime(&sat, &minuta, &sekunda);
    getDate(&godina, &mjesec, &datum, &dan);
    sprintf((char*) lstr, "%d", datum);
    strcat(lstr, "/");
    sprintf((char*) tbf, "%d", mjesec);
    strcat(lstr, tbf);
    strcat(lstr, "/");
    sprintf((char*) tbf, "%d", godina + 2000);
    strcat(lstr, tbf);
    AddTag("DATE", lstr);
    EEread(0x100 * (cnt + 1) + EE_SENSOR_NAME, lstr, 16);

    sprintf((char*) lstr, "%d", sat);
    strcat(lstr, ":");
    sprintf((char*) tbf, "%d", minuta);
    strcat(lstr, tbf);
    strcat(lstr, ":");
    sprintf((char*) tbf, "%d", sekunda);
    strcat(lstr, tbf);
    AddTag("TIME", lstr);

    for (cnt = 0; cnt < NUMBER_OF_SENSOR; cnt++)
	{
	if (senzor[cnt].ENBLE) //***** SENSOR [i]
	    {
	    if (state[cnt] == OK)
		AddTag(sStat[cnt], "OK");
	    else
		AddTag(sStat[cnt], "ERROR");
	    }
	else
	    AddTag(sStat[cnt], "DISABLE");
	EEread(0x100 * (cnt + 1) + EE_SENSOR_NAME, lstr, 16);
	AddTag(sName[cnt], lstr);
	temp_to_ascii(temp[cnt], tmpBuf);
	AddTag(sTemp[cnt], tmpBuf);
	temp_to_ascii(senzor[cnt].LOWLIMIT, tmpBuf);
	AddTag(sTLow[cnt], tmpBuf);
	temp_to_ascii(senzor[cnt].HIGHLIMIT, tmpBuf);
	AddTag(sTHigh[cnt], tmpBuf);

	if (senzor[cnt].STATE == ALARM_HI)
	    AddTag(sAlarm[cnt], "ALHIGH");
	else if (senzor[cnt].STATE == ALARM_LOW)
	    AddTag(sAlarm[cnt], "ALLOW");
	else
	    AddTag(sAlarm[cnt], "NONE");
	delaytimer = NVRAM_Read32(NV_DELAY_SEN1+(cnt*4));
	if (delaytimer)
	    {
	    delaysec = senzor[cnt].DELAY - delaytimer;
	    delayhh = delaysec / 3600;
	    delaymm = (delaysec - delayhh * 3600) / 60;
	    delayss = delaysec - delayhh * 3600 - delaymm * 60;
	    sprintf(tmpBuf, "%d:%d:%d", delayhh, delaymm, delayss);
	    }
	else
	    sprintf(tmpBuf, "0:0:0");
	AddTag(sDelay[cnt], tmpBuf);
	}

    CloseTag("ONLINE");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLOnline.xml() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira Credit.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLCredit(UINT8 *xmlBuf)
    {
    CreateXMLHeader();
    OpenTag("CREDIT");
    AddTag("STATE", infobip_credit);
    CloseTag("CREDIT");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLCredit() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira Credit.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLSmsstatus(UINT8 *xmlBuf)
    {
    CreateXMLHeader();
    getTime(&sat, &minuta, &sekunda);
    getDate(&godina, &mjesec, &datum, &dan);
    OpenTag("SMS");
    AddTag("CREDIT", infobip_credit);
    AddTag("STATE", infobip_status);
    CloseTag("SMS");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLSmsstatus() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija pokreæe proceduru oèitavanja statusa kredita									 **
 **-----------------------------------------------------------------------*/
UINT16 XMLStartCredit(UINT8 *xmlBuf)
    {
    get_credit();	//pokreæe oèitanje kredita
    strcpy(infobip_credit, "WAIT");
    CreateXMLHeader();
    OpenTag("STARTCREDIT");
    AddTag("STATE", "STARTED");
    CloseTag("STARTCREDIT");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLStartCredit() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira Sample.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLSample(UINT8 *xmlBuf)
    {
    uint16_t tmp;

    CreateXMLHeader();
    OpenTag("SAMPLE");
    tmp = EEread_8(EE_STATUS_SEND);
    if (tmp == 0)
	AddTag("CHECK", "OFF");
    else
	AddTag("CHECK", "ON");
    tmp = EEread_16(EE_STATUS_TIME);
    AddNumTag("REPORT", tmp);
    tmp = EEread_16(EE_SCAN_TIME);
    AddNumTag("LOGTIME", tmp);
    CloseTag("SAMPLE");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLSample() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira XMLHaccp.xml file									 **
 **-----------------------------------------------------------------------*/
UINT16 XMLHaccp(UINT8 *xmlBuf)
    {
    uint16_t cnt;
    int err = 0;
    CreateXMLHeader();

    OpenTag("LOGFILE");
    OpenTag("SENZOR");
    EEread(0x100 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 1
    AddTag("SEN1", (char*) tbuf);
    AddTempTag("LOW1", senzor[0].LOWLIMIT);
    AddTempTag("HIGH1", senzor[0].HIGHLIMIT);

    EEread(0x200 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 2
    AddTag("SEN2", (char*) tbuf);
    AddTempTag("LOW2", senzor[1].LOWLIMIT);
    AddTempTag("HIGH2", senzor[1].HIGHLIMIT);

    EEread(0x300 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 3
    AddTag("SEN3", (char*) tbuf);
    AddTempTag("LOW3", senzor[2].LOWLIMIT);
    AddTempTag("HIGH3", senzor[2].HIGHLIMIT);

    EEread(0x400 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 4
    AddTag("SEN4", (char*) tbuf);
    AddTempTag("LOW4", senzor[3].LOWLIMIT);
    AddTempTag("HIGH4", senzor[3].HIGHLIMIT);

    EEread(0x500 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 5
    AddTag("SEN5", (char*) tbuf);
    AddTempTag("LOW5", senzor[4].LOWLIMIT);
    AddTempTag("HIGH5", senzor[4].HIGHLIMIT);

    EEread(0x600 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 6
    AddTag("SEN6", (char*) tbuf);
    AddTempTag("LOW6", senzor[5].LOWLIMIT);
    AddTempTag("HIGH6", senzor[5].HIGHLIMIT);

    EEread(0x700 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 7
    AddTag("SEN7", (char*) tbuf);
    AddTempTag("LOW7", senzor[6].LOWLIMIT);
    AddTempTag("HIGH7", senzor[6].HIGHLIMIT);

    EEread(0x800 + EE_SENSOR_NAME, tbuf, 16);	//SENZOR 8
    AddTag("SEN8", (char*) tbuf);
    AddTempTag("LOW8", senzor[7].LOWLIMIT);
    AddTempTag("HIGH8", senzor[7].HIGHLIMIT);

    CloseTag("SENZOR");

    lfs_dir_open(&wtlfs, &tmpdir, "");
    uint32_t used_blocks = 0;

    while (true)
	{
	int res = lfs_dir_read(&wtlfs, &tmpdir, &tmpinfo);
	if (res < 0)
	    {
	    return res;
	    }

	if (res == 0)
	    {
	    break;
	    }

	if (tmpinfo.type == LFS_TYPE_REG)
	    {
	    if ((tmpinfo.name[0]) == 'H')
		{
		OpenTag("LF");
		AddTag("LOG", tmpinfo.name);
		CloseTag("LF");
		}
	    cnt++;
	    }

	}
    lfs_dir_close(&wtlfs, &tmpdir);

    CloseTag("LOGFILE");

    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);

    }/***** XMLHaccp.xml() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira pass.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLPass(UINT8 *xmlBuf)
    {
    EEread(EE_PASS, rBuf, 16);
    CreateXMLHeader();
    OpenTag("LOGIN");
    EEread(EE_USERNAME, tbuf, 16);
    AddTag("UNAME", tbuf);
    EEread(EE_PASS, tbuf, 16);
    AddTag("PASS", tbuf);
    CloseTag("LOGIN");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLSample() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira backup.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLBackup(UINT8 *xmlBuf)
    {
    unsigned char tmp;
    UINT8 cnt;
    INT8 *tbuf, slen;
    UINT32 Adresa;
    //READ_FLASH(0xF00,rBuf,256);
    CreateXMLHeader();

    OpenTag("BACKUP");
    tmp = rBuf[0x08];	//--------<ONOFF>
    if (tmp == 0)
	AddTag("ONOFF", "OFF");
    else
	AddTag("ONOFF", "ON");
    Adresa = rBuf[0x00];
    Adresa = Adresa << 8;	//-------------<IP>
    Adresa = Adresa + rBuf[0x01];
    Adresa = Adresa << 8;
    Adresa = Adresa + rBuf[0x02];
    Adresa = Adresa << 8;
    Adresa = Adresa + rBuf[0x03];
    LONGtoIP(tmpBuf, Adresa);
    AddTag("IP", tmpBuf);

    Adresa = rBuf[0x04] * 0x100 + rBuf[0x05];
    sprintf(tmpBuf, "%d", Adresa);
    AddTag("PORT", tmpBuf);	//--------<PORT>
    Adresa = rBuf[0x06] * 0x100 + rBuf[0x07];
    sprintf(tmpBuf, "%d", Adresa);
    AddTag("BACKTIME", tmpBuf);	//--------<BACKTIME>
//strcpy(twi_buf,(char const*)&rBuf[0x09]);
//AddTag("PATH",twi_buf);//--------<PATH>
    CloseTag("BACKUP");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLBackup() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira SNTP.xml file										 **
 **-----------------------------------------------------------------------*/
UINT16 XMLSNTP(UINT8 *xmlBuf)
    {
    uint8_t tmp;
    //INT8 *tbuf,slen;

    CreateXMLHeader();
    OpenTag("SNTP");
    tmp = EEread_8(EE_DST_STATE);	//--------<DST>
    if (tmp == 0)
	AddTag("DST", "OFF");
    else
	AddTag("DST", "ON");
    tmp = EEread_8(EE_SNTP_STATE);	//--------<SENAB>
    if (tmp == 0)
	AddTag("SENAB", "OFF");
    else
	AddTag("SENAB", "ON");
    tmp = EEread_8(EE_DST_ACTIVETIME);
    AddNumTag("DSTSTATE", tmp);  //zimsko ili ljetno vrijeme
    EEread(EE_SNTP1, tbuf, 32);
    AddTag("SNTP1", tbuf);
    EEread(EE_SNTP2, tbuf, 32);
    AddTag("SNTP2", tbuf);
    tmp = EEread_8(EE_TIME_ZONE);	//--------<DST>
    AddNumTag("ZONA", tmp);
    CloseTag("SNTP");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLSNTP() *****/

UINT16 XMLInput(UINT8 snum, UINT8 *xmlBuf)
    {
    uint8_t cnt;
    UINT16 itmp, tCalibCode;
    uint8_t sms1, sms2, sms3, sms4, sms5;
    uint8_t wapp1, wapp2, wapp3, wapp4, wapp5;
    CreateXMLHeader();
    OpenTag("INPUT");
    EEread(EE_INPUT_NAME1, lstr, 32);
    AddTag("NAME1", (char*) lstr);
    EEread(EE_INPUT_NAME2, lstr, 32);
    AddTag("NAME2", (char*) lstr);
    EEread(EE_INPUT_NAME3, lstr, 32);
    AddTag("NAME3", (char*) lstr);

    EEread(EE_INPUT_ALARM1, rBuf, 6);

    if (rBuf[0] == 1)
	AddTag("ALARM1", "ON");
    else
	AddTag("ALARM1", "OFF");
    if (rBuf[1] == 1)
	AddTag("ALARM2", "ON");
    else
	AddTag("ALARM2", "OFF");
    if (rBuf[2] == 1)
	AddTag("ALARM3", "ON");
    else
	AddTag("ALARM3", "OFF");

    if (rBuf[3] == 1)
	AddTag("TIP1", "NC");
    else
	AddTag("TIP1", "NO");
    if (rBuf[4] == 1)
	AddTag("TIP2", "NC");
    else
	AddTag("TIP2", "NO");
    if (rBuf[5] == 1)
	AddTag("TIP3", "NC");
    else
	AddTag("TIP3", "NO");

    EEread(EE_INPUT_SMS1, rBuf, 5);
    sms1 = rBuf[0];
    sms2 = rBuf[1];
    sms3 = rBuf[2];
    sms4 = rBuf[3];
    sms5 = rBuf[4];

    EEread(EE_INPUT_WAPP1, rBuf, 5);
    wapp1 = rBuf[0];
    wapp2 = rBuf[1];
    wapp3 = rBuf[2];
    wapp4 = rBuf[3];
    wapp5 = rBuf[4];

    if (wapp1 == 0)
	AddTag("WAPP1", "OFF");
    else
	AddTag("WAPP1", "ON");
    if (wapp2 == 0)
	AddTag("WAPP2", "OFF");
    else
	AddTag("WAPP2", "ON");
    if (wapp3 == 0)
	AddTag("WAPP3", "OFF");
    else
	AddTag("WAPP3", "ON");
    if (wapp4 == 0)
	AddTag("WAPP4", "OFF");
    else
	AddTag("WAPP4", "ON");
    if (wapp5 == 0)
	AddTag("WAPP5", "OFF");
    else
	AddTag("WAPP5", "ON");
    if (sms1 == 0)
	AddTag("SMS1", "OFF");
    else
	AddTag("SMS1", "ON");
    if (sms2 == 0)
	AddTag("SMS2", "OFF");
    else
	AddTag("SMS2", "ON");
    if (sms3 == 0)
	AddTag("SMS3", "OFF");
    else
	AddTag("SMS3", "ON");
    if (sms4 == 0)
	AddTag("SMS4", "OFF");
    else
	AddTag("SMS4", "ON");
    if (sms5 == 0)
	AddTag("SMS5", "OFF");
    else
	AddTag("SMS5", "ON");
    itmp = EEread_16(EE_INPUT_DELAY1);
    AddNumTag("DELAY1", itmp);
    itmp = EEread_16(EE_INPUT_DELAY2);
    AddNumTag("DELAY2", itmp);
    itmp = EEread_16(EE_INPUT_DELAY3);
    AddNumTag("DELAY3", itmp);

    CloseTag("INPUT");
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLInput() *****/

UINT16 XMLWapp(UINT8 snum, UINT8 *xmlBuf)
    {
    uint8_t cnt;
    UINT16 itmp, tCalibCode;
    uint8_t sms1, sms2, sms3, sms4, sms5;
    uint8_t wapp1, wapp2, wapp3, wapp4, wapp5;
    CreateXMLHeader();
    OpenTag("WHATSAPP");

    EEread(EE_WAPP1_NUM, lstr, 32);
    AddTag("NUMBER1", (char*) lstr);
    EEread(EE_WAPP1_KEY, lstr, 32);
    AddTag("KEY1", (char*) lstr);
    EEread(EE_WAPP1_NAME, lstr, 64);
    AddTag("NAME1", (char*) lstr);

    EEread(EE_WAPP2_NUM, lstr, 32);
    AddTag("NUMBER2", (char*) lstr);
    EEread(EE_WAPP2_KEY, lstr, 32);
    AddTag("KEY2", (char*) lstr);
    EEread(EE_WAPP2_NAME, lstr, 64);
    AddTag("NAME2", (char*) lstr);

    EEread(EE_WAPP3_NUM, lstr, 32);
    AddTag("NUMBER3", (char*) lstr);
    EEread(EE_WAPP3_KEY, lstr, 32);
    AddTag("KEY3", (char*) lstr);
    EEread(EE_WAPP3_NAME, lstr, 64);
    AddTag("NAME3", (char*) lstr);

    EEread(EE_WAPP4_NUM, lstr, 32);
    AddTag("NUMBER4", (char*) lstr);
    EEread(EE_WAPP4_KEY, lstr, 32);
    AddTag("KEY4", (char*) lstr);
    EEread(EE_WAPP4_NAME, lstr, 64);
    AddTag("NAME4", (char*) lstr);

    EEread(EE_WAPP5_NUM, lstr, 32);
    AddTag("NUMBER5", (char*) lstr);
    EEread(EE_WAPP5_KEY, lstr, 32);
    AddTag("KEY5", (char*) lstr);
    EEread(EE_WAPP5_NAME, lstr, 64);
    AddTag("NAME5", (char*) lstr);

    CloseTag("WHATSAPP");
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLInput() *****/

UINT16 XMLMQTT(UINT8 *xmlBuf)
    {
    uint8_t tmp;
    UINT8 cnt;
    INT8 *tbuf, slen;
    UINT32 Adresa;
    CreateXMLHeader();
    OpenTag("MQTT");
    tmp = EEread_8(EE_MQTT_ENABLE);
    if (tmp == 0)
	AddTag("ENABLE", "OFF");
    else
	AddTag("ENABLE", "ON");	//--------<ENABLE>

    EEread(EE_MQTT_SERVER, twi_buf, 64);
    AddTag("SERVER", twi_buf);	//--------<SERVER>

    Adresa = EEread_16(EE_MQTT_PORT);
    sprintf(tmpBuf, "%d", Adresa);
    AddTag("PORT", tmpBuf);	//--------<PORT>

    EEread(EE_MQTT_UNAME, twi_buf, 32);
    AddTag("USERNAME", twi_buf);	//--------<USERNAME>

    EEread(EE_MQTT_PASS, twi_buf, 32);
    AddTag("PASS", twi_buf);	//--------<PASWORD>

    EEread(EE_MQTT_TOPIC, twi_buf, 64);
    AddTag("TOPIC", twi_buf);	//--------<TOPIC>

    tmp = EEread_8(EE_MQTT_KEEPALIVE);
    sprintf(tmpBuf, "%d", tmp);
    AddTag("KEEPALIVE", tmpBuf);	//--------<KEEPALIVE>

    tmp = EEread_8(EE_MQTT_QOS);
    sprintf(tmpBuf, "%d", tmp);
    AddTag("QOS", tmpBuf);	//--------<QOS>

    tmp = EEread_8(EE_MQTT_PERIOD);
    sprintf(tmpBuf, "%d", tmp);
    AddTag("PERIOD", tmpBuf);	//--------<PERIOD>

    CloseTag("MQTT");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLMQTT() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira XMLOk.xml file									 **
 **-----------------------------------------------------------------------*/
UINT16 XMLOk(UINT8 *xmlBuf)
    {

    UINT16 ugmax;
    CreateXMLHeader();
    OpenTag("STATE");
    AddTag("OK", "ok");
    CloseTag("STATE");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLOk.xml() *****/

/*-----------------------------------------------------------------------**
 ** Funkcija kreira XMLOnline.xml file									 **
 **-----------------------------------------------------------------------*/
UINT16 XMLOnlineInput(UINT8 *xmlBuf)
    {
    UINT8 cnt;
    char tbf[8];
    uint32_t delaytimer, delaysec;
    uint16_t delayhh, delaymm, delayss;

    CreateXMLHeader();
    OpenTag("ONLINE");

    EEread(EE_INPUT_NAME1, lstr, 32);
    AddTag("IN1NAME", (char*) lstr);
    EEread(EE_INPUT_NAME2, lstr, 32);
    AddTag("IN2NAME", (char*) lstr);
    EEread(EE_INPUT_NAME3, lstr, 32);
    AddTag("IN3NAME", (char*) lstr);

    if (dinput[0].state == 0)
	AddTag("STAT1", "OPEN");
    else
	AddTag("STAT1", "CLOSE");
    if (dinput[1].state == 0)
	AddTag("STAT2", "OPEN");
    else
	AddTag("STAT2", "CLOSE");
    if (dinput[2].state == 0)
	AddTag("STAT3", "OPEN");
    else
	AddTag("STAT3", "CLOSE");

    if (dinput[0].alarm == 0)
	AddTag("ALARM1", "NONE");
    else
	AddTag("ALARM1", "ALARM");
    if (dinput[1].alarm == 0)
	AddTag("ALARM2", "NONE");
    else
	AddTag("ALARM2", "ALARM");
    if (dinput[2].alarm == 0)
	AddTag("ALARM3", "NONE");
    else
	AddTag("ALARM3", "ALARM");

    for (cnt = 0; cnt < 3; cnt++)
	{
	delaytimer = NVRAM_Read32(NV_DELAY_IN1+cnt*4);
	if (delaytimer)
	    {
	    delaysec = dinput[cnt].DELAY - delaytimer;
	    delayhh = delaysec / 3600;
	    delaymm = (delaysec - delayhh * 3600) / 60;
	    delayss = delaysec - delayhh * 3600 - delaymm * 60;
	    sprintf(tmpBuf, "%d:%d:%d", delayhh, delaymm, delayss);
	    }
	else
	    sprintf(tmpBuf, "0:0:0");
	AddTag(sDelay[cnt], tmpBuf);
	}


    CloseTag("ONLINE");
    xLen = strlen(XMLBuf);
    strcpy((char*) xmlBuf, XMLBuf);
    return strlen(XMLBuf);
    }/***** XMLOnline.xml() *****/
