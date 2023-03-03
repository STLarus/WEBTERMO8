#ifndef _1WIRE_H_
#define _1WIRE_H_


#include "pt.h"
#include "strukture.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>


extern UINT8 DS18B20Code(UINT8 * tbuf);
extern UINT16 ReadTemperature(void);
extern void temp_to_ascii(float ,char *);

	


#endif