#ifndef LOGGER_H
#define LOGGER_H



extern void CreateDatName(char *,UINT8 , UINT8 );
extern void SaveTemp(void);
extern void SaveHaccp(void);
extern void SaveAlarm(float ,UINT8 ,UINT8 );
extern char datname[];
extern void SaveLog(UINT8);
extern void SendSMSStatus(void);
#endif
