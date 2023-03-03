#ifndef SVA_TAGOVI_H
#define SVA_TAGOVI_H

//extern char SVABuf[];
extern unsigned char DS18B20CodeToArray(UINT8 *,unsigned char );
extern unsigned char StringToFile(UINT8 *,unsigned char );
extern unsigned char getChar(void);
extern unsigned char getTemp(unsigned char );
extern unsigned char CharToFile(UINT8 *);
extern unsigned int getInt(void);
extern unsigned int IntToFile(UINT8 *,UINT8 );
extern unsigned char IPtoFile(UINT8 *,UINT8 );
extern unsigned long IPtoLONG(char *);
extern INT8 XMLParser(char *,char * ,int , char *);


#endif
