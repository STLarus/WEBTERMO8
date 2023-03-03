#ifndef XML_TAGOVI_H
#define XML_TAGOVI_H

extern char XMLBuf[];
extern void ftimetostr(UINT16 , char * );
extern void fdatetostr(UINT16 , char * );
extern char isfextcsv(char *);
extern UINT8 LONGtoIP(char * , UINT32 );
extern unsigned char CreateXMLHeader(void);
extern unsigned int AddTag(const char *,char *);
extern unsigned int AddNumTag(const char *,UINT16);
extern unsigned int AddTempTag(const char *,UINT16 );
extern unsigned int OpenTag(char const * );
extern unsigned int CloseTag(char const * );
extern void addLength(unsigned int );

#endif
