#include "stm32f4xx.h"
#include "strukture.h"
#include "define.h"
#include "pt.h"
#include "timer.h"
#include "time.h"
#include "datatypes.h"
#include "system.h"
#include "udp.h"
#include "drajveri.h"
#include "logger.h"
#include "lwip/ip.h"
#include <stdio.h>
#include <string.h>
#include "sntp.h"


/***  C:\Users\Teo>ping pool.ntp.org
Pinging pool.ntp.org [178.218.172.164] with 32 bytes of data: ****/
#define SNTP_VERSION               (4/* NTP Version 4*/<<3)
#define SNTP_MODE_CLIENT            0x03
#define SNTP_LI_NO_WARNING          0x00
#define SNTP_MAX_DATA_LEN           48
#define SNTP_PORT 123

/* number of seconds between 1900 and 1970 */
#define DIFF_SEC_1900_1970         (2208988800UL)
static const uint8_t DaysInMonth[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

struct udp_pcb *sntp_pcb;
struct ip4_addr sntp_addr;
//UINT8 sntp_request [SNTP_MAX_DATA_LEN];
UINT8 sntp_recv [SNTP_MAX_DATA_LEN];
time_t ltime;
typedef struct
    {
    uint16_t year;        /* 1..4095 */
    uint8_t  month;        /* 1..12 */
    uint8_t  mday;        /* 1..31 */
    uint8_t  wday;        /* 0..6, Sunday = 0*/
    uint8_t  hour;        /* 0..23 */
    uint8_t  min;        /* 0..59 */
    uint8_t  sec;        /* 0..59 */
    uint8_t  dst;        /* 0 Winter, !=0 Summer */
    } RTC_t;
RTC_t rtc;
unsigned char tmpyear;




/*******************************************************************************
* Function Name  : isDST
* Description    : checks if given time is in Daylight Saving time-span.
* Input          : time-struct, must be fully populated including weekday
* Output         : none
* Return         : false: no DST ("winter"), true: in DST ("summer")
*  DST according to German standard
*  Based on code from Peter Dannegger found in the microcontroller.net forum.
*******************************************************************************/
UINT8 isDST( const RTC_t *t )
    {
    uint8_t wday, month;		// locals for faster access
    month = t->month;
    if( month < 3 || month > 10 )  		// month 1, 2, 11, 12
        {
        return 0;					// -> Winter
        }
    wday  = t->wday;
    if( t->mday - wday >= 25 && (wday || t->hour >= 2) )   // after last Sunday 2:00
        {
        if( month == 10 )  				// October -> Winter
            {
            return 0;
            }
        }
    else  							// before last Sunday 2:00
        {
        if( month == 3 )  				// March -> Winter
            {
            return 0;
            }
        }
    return 1;
    }



/*******************************************************************************
* Function Name  : adjustDST
* Description    : adjusts time to DST if needed
* Input          : non DST time-struct, must be fully populated including weekday
* Output         : time-stuct gets modified
* Return         : false: no DST ("winter"), true: in DST ("summer")
*  DST according to German standard
*  Based on code from Peter Dannegger found in the mikrocontroller.net forum.
*******************************************************************************/
UINT8 adjustDST( RTC_t *t )
    {
    uint8_t hour, day, wday, month;			// locals for faster access
    hour  = t->hour;
    day   = t->mday;
    wday  = t->wday;
    month = t->month;
    if ( isDST(t) )
        {
        EEwrite_8(EE_DST_ACTIVETIME,1);
        t->dst = 1;
        hour++;								// add one hour
        if( hour == 24 ) 					// next day
            {
            hour = 0;
            wday++;							// next weekday
            if( wday == 7 )
                {
                wday = 0;
                }
            if( day == DaysInMonth[month-1] )  		// next month
                {
                day = 0;
                month++;
                }
            day++;
            }
        t->month = month;
        t->hour  = hour;
        t->mday  = day;
        t->wday  = wday;
        return 1;
        }
    else
        {
        EEwrite_8(EE_DST_ACTIVETIME,0);
        t->dst = 0;
        return 0;
        }
    }



/********************************************************************************************************/
void sntp_set_system_time(time_t ltime)
/********************************************************************************************************/
    {
    uint32_t rectime;
    uint8_t timezone;
    uint16_t shift_time;


    //sntp_stop();

    timezone= EEread_8(EE_TIME_ZONE);
    if(timezone>20)
        {
        shift_time=(timezone-20)*3600;
        ltime=ltime-shift_time;
        }
    else
        {
        shift_time=timezone*3600;
        ltime=ltime+shift_time;
        }
    timeinfo = localtime (&ltime);
    timeinfo->tm_mon++;//mjesec pove�ati za jedan
    timeinfo->tm_year+=1900;//godina + 1900

    rtc.year=timeinfo->tm_year;
    rtc.month=timeinfo->tm_mon;
    rtc.mday=timeinfo->tm_mday;
    rtc.hour=timeinfo->tm_hour;
    rtc.min=timeinfo->tm_min;
    rtc.wday=timeinfo->tm_wday ;
    rtc.sec=timeinfo->tm_sec;

    timezone=EEread_8(EE_DST_STATE);
    if(timezone==1)
        adjustDST(&rtc);//ako je dozvoljen DST
    printf("------ podeseno -----\n");
    setDate (rtc.year-2000,rtc.month,rtc.mday,rtc.wday);
    setTime(rtc.hour,rtc.min,rtc.sec);

    }/**** sntp_receive() *****/




