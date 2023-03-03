#include "stm32f4xx_conf.h"

unsigned int Timex;
uint32_t LocalTime,sysTyck;
struct timer
	{
	unsigned int start;
	unsigned int interval;
	};

//------------------------------------------------------------------------
int timer_expired(struct timer *t)
{
return (int)(Timex - t->start) >= (int)t->interval;
} /**** timer_expired() ****/



//------------------------------------------------------------------------
void timer_set(struct timer *t, unsigned int interval)
{
t->interval = interval;
t->start = Timex;
}/**** timer_set() ***/

void IncrementTimer(void)
{
Timex++;
LocalTime=LocalTime+10;
sysTyck++;
}

uint32_t sys_now(void)
{
 return sysTyck;
}

