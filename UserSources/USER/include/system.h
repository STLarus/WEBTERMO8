#ifndef SYSTEM_H_INCLUDE
#define SYSTEM_H_INCLUDE


#include "datatypes.h"


/* Boolean	values*/
#define TRUE  1	/**< Boolean TRUE value as used in the OpenTCP */
#define FALSE 0	/**< Boolean FALSE value as used in the OpenTCP */



/* System functions	*/

extern void kick_WD(void);
extern void wait(INT16);
extern void enter_power_save(void);
extern void exit_power_save(void);
extern INT16 bufsearch(UINT8*, UINT16, UINT8*);
extern UINT16 hextoascii(UINT8);
extern UINT8 asciitohex(UINT8);
extern UINT8 isnumeric(UINT8);
extern void mputs(UINT8 const *);
void mputhex(UINT8 );
extern void dummy(void);
extern WORD Base64Encode(BYTE* , WORD , BYTE* , WORD );
extern WORD Base64Decode(BYTE* , WORD , BYTE* , WORD );
extern unsigned char https_read_encoded2(char *);
extern char * my_ftoa(double, char * , int );
//extern void itoa(char *,int);
//extern uint8_t checkAuth(char * );

/*	External functions	*/

extern void init(void);

#endif




