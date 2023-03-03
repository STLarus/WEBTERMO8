#ifndef HTTP_CLIEN_H
#define HTTP_CLIENT_H



extern void WCLinit(void);
extern void wcl_poll(void);
extern void send_infobip(UINT8 ,UINT8 ,UINT8 ,UINT8 ,UINT8);
extern void get_credit(void);
#endif