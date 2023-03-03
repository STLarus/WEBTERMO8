#ifndef DRIVER_H
#define DRIVER_H


extern void delay_us(unsigned int);
extern void delay_ms(unsigned int);
extern void getTime(unsigned char *, unsigned char *, unsigned char *);
extern void getDate (unsigned char *, unsigned char *, unsigned char *,unsigned char *);
extern void setTime (unsigned char , unsigned char , unsigned char );
extern void setDate (unsigned char , unsigned char, unsigned char, unsigned char );
extern void getPointer(unsigned long *,unsigned long *);
extern void setPointer(unsigned long, unsigned long );
extern void setLRead (unsigned long );
extern void getLRead (unsigned long *);
extern void setLSave (unsigned long );
extern void getLSave (unsigned long *);
extern unsigned char read24AA02(unsigned char );
extern void write24AA02(unsigned char ,unsigned char ,unsigned char*);
extern unsigned long fnNumData(void);
extern void InitDS1307(void);
extern void ResetRDR(void);
extern void I2CInit(void);
extern void DS2482_busywait(unsigned char);
extern unsigned char OWreset(unsigned char);
extern void OWWriteByte(unsigned char,unsigned char);
extern void OWReadByte(unsigned char,unsigned char *);
extern unsigned char DS2482_reset(unsigned char);
extern unsigned char DS2482_init(unsigned char );
extern void I2Cinit(void);
extern void getTimeASCII(char *);
extern void getDateASCII(char *);
#endif

