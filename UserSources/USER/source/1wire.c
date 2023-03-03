#include "stm32f4xx.h"
#include "pt.h"
#include "strukture.h"
#include "logger.h"
#include "define.h"
#include "timer.h"
#include "datatypes.h"
#include "drajveri.h"
#include "strukture.h"
#include "system.h"
#include "define.h"
#include <stdio.h>
#include <string.h>



struct timer SaveTimer,ReadTimer;
struct pt pt_ReadTemp,pt_SaveTemp;



#define DSREAD_TIME	1000	//vrijeme Ã¨ekanja na kraj konverzije u ms; ujedno i vrijem skeniranja po senzoru
#define	ONEWIRE_ERROR_COUNT	5	//broj pogreÅ¡nih oÃ¨itavanja zaredom prije nego se proglasi alarm

#define     OWI_CRC_OK      0x00    //!< CRC check succeded
#define     OWI_CRC_ERROR   0x01    //!< CRC check failed
extern void gisAlarm(uint8_t sid,uint8_t atype,float avalue);
extern char twi_buf[];
extern void Delay(uint32_t);
unsigned char templow,temphigh;
uint8_t alarm,dsBuf[10];
const char * const decTab[] =
    {
    ",00",",06",",13",",19",",25",",31",",38",",44",
    ",50",",56",",63",",69",",75",",81",",88",",94",
    };

/*! \brief  Compute the CRC8 value of a data set.
 *
 *  This function will compute the CRC8 or DOW-CRC of inData using seed
 *  as inital value for the CRC.
 *
 *  \param  inData  One byte of data to compute CRC from.
 *
 *  \param  seed    The starting value of the CRC.
 *
 *  \return The CRC8 of inData with seed as initial value.
 *
 *  \note   Setting seed to 0 computes the crc8 of the inData.
 *
 *  \note   Constantly passing the return value of this function
 *          As the seed argument computes the CRC8 value of a
 *          longer string of data.
 */
unsigned char OWI_ComputeCRC8(unsigned char inData, unsigned char seed)
    {
    unsigned char bitsLeft;
    unsigned char temp;

    for (bitsLeft = 8; bitsLeft > 0; bitsLeft--)
        {
        temp = ((seed ^ inData) & 0x01);
        if (temp == 0)
            {
            seed >>= 1;
            }
        else
            {
            seed ^= 0x18;
            seed >>= 1;
            seed |= 0x80;
            }
        inData >>= 1;
        }
    return seed;
    }



/*! \brief  Calculate and check the CRC of a 64 bit ROM identifier.
 *
 *  This function computes the CRC8 value of the first 56 bits of a
 *  64 bit identifier. It then checks the calculated value against the
 *  CRC value stored in ROM.
 *
 *  \param  romvalue    A pointer to an array holding a 64 bit identifier.
 *
 *  \retval OWI_CRC_OK      The CRC's matched.
 *  \retval OWI_CRC_ERROR   There was a discrepancy between the calculated and the stored CRC.
 */
unsigned char OWI_CheckRomCRC(unsigned char * romValue)
    {
    unsigned char i;
    unsigned char crc8 = 0;

    for (i = 0; i < 7; i++)
        {
        crc8 = OWI_ComputeCRC8(*romValue, crc8);
        romValue++;
        }
    if (crc8 == (*romValue))
        {
        return OWI_CRC_OK;
        }
    return OWI_CRC_ERROR;
    }

/*! \brief  Provjerava da li je pojedini senzor u alarmnom stanju.
 *
 *  \param  temp    Trenutna oÃ¨itana temperature
 *  \param  highlimit   Gornja granica temperature
 *  \param  lowlimit    Donja granica temperature
 *  \param  hist	   Histereza
 *  \param  *state	   pointer na ternutno stanje alarma
 *					0..nije doÅ¡lo do promjene alarma 1...nastupio alarm
 *  \param  snum	   Redni broj senzora
 *
 *  \retval stanje alarma NO_ALARM...0;   ALARM_HI....1;	ALARM_LOW....2
 */
UINT16 test_alarm(float temp,INT8 highlimit, INT8 lowlimit, UINT8 hist,UINT8 *state, UINT8 snum)
    {
    //int SignBit,Treading;
    //UINT16 tpt;
    UINT16 retval=0;
    float fTemp,fHlimit=0,fLlimit=0;
    UINT8 oldstate;
    oldstate=*state;

    fHlimit=(float)highlimit;
    fLlimit=(float)lowlimit;

    fTemp=temp;
    if(*state==NO_ALARM)
        {
        if(fTemp>=(fHlimit+hist))
            {
            retval=1;
            *state=ALARM_HI;
            }
        else if(fTemp<=(fLlimit-hist))
            {
            retval=2;
            *state=ALARM_LOW;
            }
        }
    else if (*state==ALARM_HI)
        {
        if(fTemp<=(fHlimit-hist))
            *state=NO_ALARM;
        }
    else if (*state==ALARM_LOW)
        {
        if(fTemp>=(fLlimit+hist))
            *state=NO_ALARM;
        }
    if(oldstate!=*state)
        {
        oldstate=*state;
        if(*state==NO_ALARM)
            oldstate=TEMPNORMAL;
        else if(*state==ALARM_HI)
            oldstate=TEMPHIGH;
        else if(*state==ALARM_LOW)
            oldstate=TEMPLOW;
//        gisAlarm(snum+1,oldstate,fTemp);
        }

    return retval;
    }/***** test_alarm() *****/



/*! \brief  Pretvara temperaturu iz DS18B20 formata u ASCII.
*
*  Ova funkcija uzima temperaturu iz DS18B20 formata
*  i pretvara u ASCII zapis na dvije decimale npr 18.32
*
*  \param  rtemp    Trenutna oÃ¨itana temperature
*  \param  ascbuf   Pointer na buffer u koji se sprema tenmperatura
*
*  \retval Nema
*/

void temp_to_ascii(float rtemp,char *ascbuf)
    {
    my_ftoa(rtemp, ascbuf, 2);
    }/***** temp_to_ascii() *****/



void test_ds2482(void)
    {
    int res,temperature;
    unsigned char buf[10];
    UINT8 channel=4;
    DS2482_reset(channel);
    OWreset(channel);

    //**	***	read code
    OWreset(channel);
    OWWriteByte(channel,0x33); //READ ROM
    for(res=0; res<8; res++)
        OWReadByte(channel,&buf[res]);
    res=OWI_CheckRomCRC(&buf[0]);
    //*****	read temperature
    OWreset(channel);
    OWWriteByte(channel,0xCC);
    OWWriteByte(channel,0x44);	//CONVERT
    //vrijeme konverzije od 1000 ms ne radi, treba izmjeriti tajmer koliko je toÃ¨an
    delay_ms(500);	//500*10ms
    OWreset(channel);
    OWWriteByte(channel,0xCC);	//skip ROM
    OWWriteByte(channel,0xBE);	//
    OWReadByte(channel,&templow);
    OWReadByte(channel,&temphigh);
    if(I2C_error==0)
        {
        //ako je greÅ¡ka ne zapisuje temperaturu
        temperature=(templow & 0x000F);
        temperature=temphigh*256+templow;
        temperature=(temperature>>4) & 0x007F;
        }
    }


UINT8 DS18B20Code(UINT8 * tbuf)
    {
    UINT8 res;
    OWreset(1);
    OWWriteByte(1,0x33); //READ ROM
    for(res=0; res<8; res++)
        OWReadByte(1,&tbuf[res]);
    res=OWI_CheckRomCRC(&tbuf[0]);
    return res;
    }/***** DS18B20Code() *****/

UINT8 calc_crc(UINT8 buff[], UINT8 num_vals)
    {
    UINT8 shift_reg=0, data_bit, sr_lsb, fb_bit, i, j;

    for (i=0; i<num_vals; i++) /* for each byte */
        {
        for(j=0; j<8; j++)   /* for each bit */
            {
            data_bit = (buff[i]>>j)&0x01;
            sr_lsb = shift_reg & 0x01;
            fb_bit = (data_bit ^ sr_lsb) & 0x01;
            shift_reg = shift_reg >> 1;
            if (fb_bit)
                {
                shift_reg = shift_reg ^ 0x8c;
                }
            }
        }
    return(shift_reg);
    }



PT_THREAD(ptReadTemp(struct pt *pt))
    {
    static UINT8 cnt,k,chk_sum;
    static UINT8 channel;
    int SignBit;
    float fTemp;
    UINT16 readtemp;
    uint32_t temp32;
    PT_BEGIN(pt);
    PT_YIELD(pt);
    for(k=0; k<NUMBER_OF_SENSOR; k++)
        {
        if(senzor[k].ENBLE)
            {
            channel=senzor[k].KANAL;
            OWreset(channel);
            OWWriteByte(channel,0x55);	//MATCH ROM
            for(cnt=0; cnt<8; cnt++)
                OWWriteByte(channel,senzor[k].CODE[cnt]);
            OWWriteByte(channel,0x44);	//CONVERT
            timer_set(&ReadTimer,DSREAD_TIME);
            PT_WAIT_UNTIL(pt,timer_expired(&ReadTimer));
            OWreset(channel);
            OWWriteByte(channel,0x55);	//MATCH ROM
            for(cnt=0; cnt<8; cnt++)
                OWWriteByte(channel,senzor[k].CODE[cnt]);
            OWWriteByte(channel,0xBE);	//
            for(cnt=0; cnt<9; cnt++)
                OWReadByte(channel,&dsBuf[cnt]);

            chk_sum=calc_crc(&dsBuf[0], 8);
// VERZIJA (teovu#1#): Verzija 1.4, ispitivanje reseta senzora zbog povremene pojave temperature od 85C

            if(dsBuf[0]==0x50)
                {
                if(dsBuf[1]==0x05)
                    chk_sum++; //umjetno izazavana greska ocitanja
                }
//--------------------------------------------------------------------------------
            if(chk_sum==dsBuf[8])
                {
                templow=dsBuf[0];
                temphigh=dsBuf[1];
                readtemp=temphigh*256+templow;
                SignBit=readtemp & 0x8000;
                if(SignBit)
                    readtemp=(readtemp ^ 0xffff) + 1;
                fTemp = (float) (6 * readtemp) + readtemp / 4;  // Multiply by (100 * 0.0625) or 6.25
                fTemp = fTemp/100;
                if(SignBit)
                    fTemp=0-fTemp;


                //dodaje se kalibracija
                temp[k]=fTemp+senzor[k].kalibracija;
                //upisivanje u NVRAM za izraÄ�un srednje vrijednopsti
                fTemp=NVRAM_ReadFloat(NV_SUM_SEN1+(k*8));
                fTemp+=temp[k];
                NVRAM_WriteFloat(NV_SUM_SEN1+(k*8),fTemp);
                temp32=NVRAM_Read32(NV_COUNT_SEN1+(k*8));
                temp32++;
                NVRAM_Write32(NV_COUNT_SEN1+(k*8),temp32);


                senzor[k].error_counter=0;
                state[k]=OK;
                }
            else
                {
                if(senzor[k].error_counter<ONEWIRE_ERROR_COUNT)
                    senzor[k].error_counter++;
                state[k]=KO;
                }
            }
        else
            {
            state[k]=DISABLE;
            state[k]=0;
            }
        }
    for(k=0; k<NUMBER_OF_SENSOR; k++)
        {
        if(senzor[k].ENBLE)//ispitivanje alarma
            {
            alarm=0;
            if(senzor[k].error_counter==0)//ispituje alarm samo ako nema greÅ¡ke
                alarm=test_alarm(temp[k],senzor[k].HIGHLIMIT,senzor[k].LOWLIMIT,senzor[k].HIS,&senzor[k].STATE,k);
            else if(senzor[k].error_counter==ONEWIRE_ERROR_COUNT)
                {
                alarm=ALARM_ERR;
                senzor[k].error_counter +=2;
                }
            if(alarm)
                SaveAlarm(temp[k],k,alarm);
            }
        }
    PT_END(pt);
    }/***** ptReadTemp() *****/

PT_THREAD(ptSaveTemp(struct pt *pt))
    {
    PT_BEGIN(pt);
    PT_YIELD(pt);
    PT_WAIT_UNTIL(pt,scanflag==1);
    scanflag=0;
    SaveTemp();
    PT_END(pt);
    }/***** ptSaveTemp() *****/



UINT16 ReadTemperature(void)
    {
    templow=0xFF;
    temphigh=0xFF;
    ptReadTemp(&pt_ReadTemp);
    ptSaveTemp(&pt_SaveTemp);
    return (temphigh*256+templow);
    }/***** ReadTemperature() *****/

