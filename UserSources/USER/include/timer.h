#ifndef TIMER_H
#define TIMER_H

#include "stm32f4xx_conf.h"

struct timer{unsigned int start, interval;};
extern unsigned int Timex;
extern uint32_t LocalTime;
extern unsigned int timer_expired(struct timer *t);
extern void timer_set(struct timer *t, unsigned int usecs);
#endif



