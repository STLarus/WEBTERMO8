/*
 * inputs.c
 *
 *  Created on: 3. kol 2023.
 *      Author: teovu
 */

#include "pt.h"
#include "timer.h"
#include "define.h"
#include "strukture.h"

struct timer Input1Timer, Input2Timer, Input3Timer;

struct pt pt_Input1NO, pt_Input1NC;
struct pt pt_Input2NO, pt_Input2NC;
struct pt pt_Input3NO, pt_Input3NC;

//#define NO	1

#define DigitalInput1()	GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9)
#define DigitalInput2()	GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10)
#define DigitalInput3()	GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11)

uint8_t	DIN1,DIN2,DIN3;

PT_THREAD( ptInput1NC(struct pt*));
PT_THREAD( ptInput1NO(struct pt*));


void ScanInputs(void)
    {

    DIN1 = DigitalInput1();
    DIN2 = DigitalInput2();
    DIN3 = DigitalInput3();

    dinput[0].state = DIN1;
    dinput[1].state = DIN2;
    dinput[2].state = DIN3;

    if (dinput[0].enable == 1 && dinput[0].type == 2)
	ptInput1NO(&pt_Input1NO);
    else if (dinput[0].enable == 1 && dinput[0].type == 1)
	ptInput1NC(&pt_Input1NO);
    }


PT_THREAD(ptInput1NC(struct pt *pt))
    {
    PT_BEGIN(pt)
    ;
    PT_YIELD(pt);
    PT_WAIT_UNTIL(pt, DIN1 == 0);

    timer_set(&Input1Timer, 100);	//vrijeme debounca
    PT_WAIT_UNTIL(pt, timer_expired(&Input1Timer));
    if (DIN1 == 0)
	{
	dinput[0].alarm = 1;
	NVRAM_Write32(NV_DELAY_IN1, 1);
	//SaveAlarm(temp[k], k, alarm);
	}
    else
	PT_RESTART(pt);
    PT_WAIT_UNTIL(pt, DIN1 > 0);
    timer_set(&Input1Timer, 100);
    PT_WAIT_UNTIL(pt, timer_expired(&Input1Timer));
    if (DIN1 > 0)
	{
	dinput[0].alarm = 0;
	NVRAM_Write32(NV_DELAY_IN1, 0);
	}
    else
	PT_RESTART(pt);
PT_END(pt);
}

PT_THREAD(ptInput1NO(struct pt *pt))
{
PT_BEGIN(pt)
;
PT_YIELD(pt);
PT_WAIT_UNTIL(pt, DIN1 > 0);
timer_set(&Input1Timer, 100);	//vrijeme debounca
PT_WAIT_UNTIL(pt, timer_expired(&Input1Timer));
if (DIN1 > 0)
    {
    dinput[0].alarm = 1;
    NVRAM_Write32(NV_DELAY_IN1, 1);
    //SaveAlarm(temp[k], k, alarm);
    }
else
    PT_RESTART(pt);
PT_WAIT_UNTIL(pt, DIN1 == 0);
timer_set(&Input1Timer, 100);
PT_WAIT_UNTIL(pt, timer_expired(&Input1Timer));
if (DIN1 == 0)
    {
    dinput[0].alarm = 0;
    NVRAM_Write32(NV_DELAY_IN1, 0);
    }
else
    PT_RESTART(pt);
PT_END(pt);
}

