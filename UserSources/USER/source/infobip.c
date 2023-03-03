//*****************************************************************************
//
// wcl.c - Smtp client for sending ewcl.
//
//
//*****************************************************************************

#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "pt.h"
#include "pt.h"
#include "timer.h"
#include "strukture.h"
#include "drajveri.h"
#include "define.h"
#include "XMLTagovi.h"
#include "SVATagovi.h"
#include <string.h>
#include <stdio.h>

#define SWAP_UINT32(w) (((w) & 0xff) << 8) | (((w) & 0xff00) >> 8)


#define WCL_WAIT_TIME	10000	//60000
#define WCL_WAITING	0
#define WCL_CONNECTED	1
#define WCL_READY	2
#define WCL_ACKED	3
#define WCL_ANSWER	4
#define ENCODE_BUF_LEN	1024
extern void rfc1738_escape(char *,char *,int);
char gsmState,gsmCreditState;
struct timer WCLTimer,WCLCreditTimer;
struct pt pt_WCL,pt_WCLcredit;
struct ip4_addr wcl_ip;
unsigned int wcl_port=80;
//unsigned int wcl_port=8080;
err_t wclresult;
struct tcp_pcb *wcl_pcb,*wcl_status_pcb;
//char indata[1024];
char wcl_active=0,wcl_credit_active=0;
UINT fwcl_rval;
char wtxBuf[1024] __attribute__ ((section(".ccmram")));
char infobip[ENCODE_BUF_LEN] __attribute__ ((section(".ccmram")));
char ibipretbuf[1024] __attribute__ ((section(".ccmram")));
char CRBuf[1024] __attribute__ ((section(".ccmram")));
static long ptr;

int infobiplen,xmlpos;

//FIL wclfp;
//FRESULT wres;
UINT fwcl_rval;

static err_t wclConnected(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t wclRecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t wclCreditRecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);



//*****************************************************************************
//
// wclient close cnnection
//
//*****************************************************************************
void wcl__close(void)
    {
//if(logfile)
//	{
//	strcpy(wtxBuf,"--------   ERROR  close wcl --------\r\n\r\n\r\n\r\n");
//	fwclresult=f_write(&wclfp,wtxBuf,strlen(wwtxBuf),&fwcl_rval);
//	f_sync(&wclfp)
//	}
//f_close(&wclfp);
    tcp_arg(wcl_pcb, NULL);
    tcp_sent(wcl_pcb, NULL);
    tcp_recv(wcl_pcb, NULL);
    tcp_close(wcl_pcb);
    wcl_active=0;
    }/**** wcl__colose() ****/


void wcl_status_close(void)
    {
    tcp_arg(wcl_status_pcb, NULL);
    tcp_sent(wcl_status_pcb, NULL);
    tcp_recv(wcl_status_pcb, NULL);
    tcp_close(wcl_status_pcb);
    wcl_credit_active=0;
    }/**** wcl_status_close() ****/


//*****************************************************************************
//
// Error trap
//
//*****************************************************************************
void WCLerror(void *arg, err_t err)
    {
    }
//*****************************************************************************
//
// Error trap
//
//*****************************************************************************
static err_t WCLsent(void *arg, struct tcp_pcb *pcb,u16_t len)
    {
    LWIP_UNUSED_ARG(arg);
    return ERR_OK;
    }

static err_t wclCreditSent(void *arg, struct tcp_pcb *pcb,u16_t len)
    {
    LWIP_UNUSED_ARG(arg);
    return ERR_OK;
    }

//*****************************************************************************
//
// tcp_connect call-back
//
//*****************************************************************************
static err_t wclConnected(void *arg, struct tcp_pcb *pcb, err_t err)
    {
    if (err == ERR_OK)
        {
        tcp_arg(pcb, NULL);
        tcp_sent(pcb, WCLsent);
        tcp_recv(pcb, wclRecv);
        gsmState=WCL_CONNECTED;
        }
    return err;
    }

static err_t wclStatusConnected(void *arg, struct tcp_pcb *pcb, err_t err)
    {
    if (err == ERR_OK)
        {
        tcp_arg(pcb, NULL);
        tcp_sent(pcb, wclCreditSent);
        tcp_recv(pcb, wclCreditRecv);
        gsmCreditState=WCL_CONNECTED;
        }
    return err;
    }

//*****************************************************************************
//
// tcp_receive call-back
//
//*****************************************************************************
static err_t wclRecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
    {
    int i;
    int lupAgain;
    char *pc;
    err_t sock_err=0;
    if (err == ERR_OK && p != NULL)
        {
        tcp_recved(pcb, p->tot_len);  /* Inform TCP that we have taken the data. */
        do
            {
            pc=(char *)p->payload;   //pointer to the pay load
            for (i=0; i < p->len; i++)//copy to our own buffer
                ibipretbuf[ptr+i]= pc[i];
            ptr+=p->len;
            lupAgain= p->next ? 1:0;	// test for null
            p= p->next;					// point to the next pbuf
            }
        while(lupAgain);
        gsmState=WCL_ANSWER;
        XMLParser("<status>",ibipretbuf,p->len,infobip_status);
        XMLParser("<credits>",ibipretbuf,p->len,infobip_credit);
        }
//if(logfile)
//	{
//	fwclresult=f_write(&wclfp,ibipretbuf,strlen(ibipretbuf),&fwcl_rval);
//	f_sync(&wclfp);
//	}
    return sock_err;
    }


static err_t wclCreditRecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
    {
    int i,reclen;
    int lupAgain;
    char *pc;
    err_t sock_err=0;

    if (err == ERR_OK && p != NULL)
        {
        tcp_recved(pcb, p->tot_len);  /* Inform TCP that we have taken the data. */
        reclen=p->tot_len;
        memcpy(ibipretbuf,(char *)p->payload,reclen);
        ibipretbuf[reclen]=0;
        gsmCreditState=WCL_ANSWER;
        for(i=reclen; i>0; i--)
            {
            //rutina ide od zadnjeg karaktera
            if(ibipretbuf[i]==0x0A)	//i tra�i zadnji LF
                break;	//sve iza toga je poruka
            }
        infobip_credit[0]=0;
        //kopira ostatak pristiglih podataka u credit buffer
        strcpy(infobip_credit,&ibipretbuf[i+1]);
        }
    return sock_err;
    }/***** wclStatusRecv() *****/

void WCLinit(void)
    {}/***** WClinit() *****/



int CrateInfobipXMLString(char *infobip)
    {
    XMLBuf[0]=0;
    strcpy(infobip,"XML=");
    OpenTag("SMS");
    strcat(XMLBuf,"\r\n");
    OpenTag("authentification");
    strcat(XMLBuf,"\r\n");

    EEread(EE_GSM_UNAME,rBuf,16);
    AddTag("username",(char*)rBuf);
    strcat(XMLBuf,"\r\n");
    strcat(XMLBuf,"\r\n");
    EEread(EE_GSM_PASS,rBuf,16);
    AddTag("password",(char*)rBuf);
    strcat(XMLBuf,"\r\n");
    strcat(XMLBuf,"\r\n");
    CloseTag("authentification");
    strcat(XMLBuf,"\r\n");
    OpenTag("message");
    strcat(XMLBuf,"\r\n");

    EEread(EE_OBJECT_NAME,rBuf,32);
    AddTag("sender",(char*)rBuf);
    strcat(XMLBuf,"\r\n");
    AddTag("text",infobip_poruka);
    strcat(XMLBuf,"\r\n");
    CloseTag("message");
    strcat(XMLBuf,"\r\n");
    OpenTag("recipients");
    strcat(XMLBuf,"\r\n");
    if(sms.gsm1_enable)
        {
        EEread(EE_GSM1_NUM,rBuf,32);
        AddTag("gsm",(char*)rBuf);
        strcat(XMLBuf,"\r\n");
        }
    if(sms.gsm2_enable)
        {
        EEread(EE_GSM2_NUM,rBuf,32);
        AddTag("gsm",(char*)rBuf);
        strcat(XMLBuf,"\r\n");
        }
    if(sms.gsm3_enable)
        {
        EEread(EE_GSM3_NUM,rBuf,32);
        AddTag("gsm",(char*)rBuf);
        strcat(XMLBuf,"\r\n");
        }
    if(sms.gsm4_enable)
        {
        EEread(EE_GSM4_NUM,rBuf,32);
        AddTag("gsm",(char*)rBuf);
        strcat(XMLBuf,"\r\n");
        }
    if(sms.gsm5_enable)
        {
        EEread(EE_GSM5_NUM,rBuf,32);
        AddTag("gsm",(char*)rBuf);
        strcat(XMLBuf,"\r\n");
        }
    CloseTag("recipients");
    strcat(XMLBuf,"\r\n");
    CloseTag("SMS");
    rfc1738_escape(XMLBuf,&infobip[4],ENCODE_BUF_LEN);
    return (strlen(infobip));
    }/***** CrateInfobipXMLString() *****/




PT_THREAD(ptWCL(struct pt *pt))
    {
    uint32_t local_ip,u32tmp;
    PT_BEGIN(pt);
    PT_YIELD(pt);

    infobiplen=CrateInfobipXMLString(infobip);
    u32tmp=EEread_32(EE_GSM_IP);
    local_ip = ((u32tmp>>24)&0xff) | // move byte 3 to byte 0
               ((u32tmp<<8)&0xff0000) | // move byte 1 to byte 2
               ((u32tmp>>8)&0xff00) | // move byte 2 to byte 1
               ((u32tmp<<24)&0xff000000); // byte 0 to byte 3
    ip4_addr_set_u32(&wcl_ip,local_ip);

    wcl_port=EEread_16(EE_GSM_PORT);//IP4_ADDR(&wcl_ip,192,168,1,20);

    XMLBuf[0]=0;
    strcpy(XMLBuf,"POST /api/sendsms/xml HTTP/1.1\r\n");
    strcat(XMLBuf,"Host: api2.infobip.com\r\n");
    strcat(XMLBuf,"Accept: */*\r\n");
    strcat(XMLBuf,"Content-length: ");
    xmlpos=strlen(XMLBuf);
    sprintf((char*)&(XMLBuf[xmlpos]),"%d",infobiplen);
    strcat(XMLBuf,"\r\n");
    strcat(XMLBuf,"Content-Type: application/x-www-form-urlencoded\r\n");
    strcat(XMLBuf,"\r\n");
    wtxBuf[0]=0;
    strcpy(wtxBuf,XMLBuf);
    wcl_pcb=tcp_new();
    tcp_bind(wcl_pcb, IP_ADDR_ANY, 0);
    timer_set(&WCLTimer,WCL_WAIT_TIME);
    gsmState=0;
    ptr=0;
    wclresult= tcp_connect(wcl_pcb, &wcl_ip, wcl_port, wclConnected);
    PT_WAIT_UNTIL(pt,timer_expired(&WCLTimer) || gsmState==WCL_CONNECTED);
    if(timer_expired((&WCLTimer)))
        {
        wcl__close();
        PT_EXIT(pt);
        }
//�alje zaglavlje
    gsmState=0;
    timer_set(&WCLTimer,WCL_WAIT_TIME);
    ptr=0;
    tcp_write(wcl_pcb,wtxBuf,strlen(wtxBuf), 0);
    tcp_output(wcl_pcb);
    timer_set(&WCLTimer,50);
    PT_WAIT_UNTIL(pt,timer_expired(&WCLTimer));
    tcp_write(wcl_pcb,infobip,strlen(infobip), 0);
    tcp_output(wcl_pcb);
    timer_set(&WCLTimer,WCL_WAIT_TIME);
    PT_WAIT_UNTIL(pt,timer_expired(&WCLTimer) || gsmState==WCL_ANSWER);
    if(timer_expired((&WCLTimer)))
        {
        wcl__close();
        PT_EXIT(pt);
        }

//ovdje treba obraditi odgovor koji stigne
    wcl__close();
    wcl_active=0;
    PT_END(pt);
    }/***** ptWCL() *****/


PT_THREAD(ptWCLcredit(struct pt *pt))
    {
    uint32_t local_ip,u32tmp;
    PT_BEGIN(pt);
    PT_YIELD(pt);

    infobiplen=CrateInfobipXMLString(infobip);
    local_ip=EEread_32(EE_GSM_IP);
    u32tmp=EEread_32(EE_GSM_IP);
    local_ip = ((u32tmp>>24)&0xff) | // move byte 3 to byte 0
               ((u32tmp<<8)&0xff0000) | // move byte 1 to byte 2
               ((u32tmp>>8)&0xff00) | // move byte 2 to byte 1
               ((u32tmp<<24)&0xff000000); // byte 0 to byte 3
    ip4_addr_set_u32(&wcl_ip,local_ip);

    wcl_port=EEread_16(EE_GSM_PORT);

    CRBuf[0]=0;


    AddTag("PASS",tbuf);
    strcpy(CRBuf,"GET /api/command?username=");
    EEread(EE_GSM_UNAME,rBuf,16);
    strcat(CRBuf,rBuf);
    strcat(CRBuf,"&password=");
    EEread(EE_GSM_PASS,rBuf,16);
    strcat(CRBuf,rBuf);
    strcat(CRBuf,"&cmd=CREDITS HTTP/1.1\r\n");
    strcat(CRBuf,"Host: api.infobip.com\r\n");
    strcat(CRBuf,"Connection: keep-alive\r\n");
    strcat(CRBuf,"Cache-Control: no-cache\r\n");
    strcat(CRBuf,"User-Agent: Larus web server\r\n");
    strcat(CRBuf,"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n");
    strcat(CRBuf,"Accept-Encoding: gzip,deflate\r\n");
    strcat(CRBuf,"Accept-Language: hr-HR,hr;q=0.8,en-US;q=0.6,en;q=0.4\r\n");
    strcat(CRBuf,"Accept-Charset: windows-1250,utf-8;q=0.7,*;q=0.3\r\n");
    strcat(CRBuf,"\r\n");
    strcpy(infobip_credit,"WAIT");  //inicijalizacija statusa
    wcl_status_pcb=tcp_new();
    tcp_bind(wcl_status_pcb, IP_ADDR_ANY, 0);
    gsmCreditState=0;
    ptr=0;
    timer_set(&WCLCreditTimer,WCL_WAIT_TIME);
    wclresult= tcp_connect(wcl_status_pcb, &wcl_ip, wcl_port, wclStatusConnected);
    PT_WAIT_UNTIL(pt,timer_expired(&WCLCreditTimer) || gsmCreditState==WCL_CONNECTED);
    if(timer_expired((&WCLCreditTimer)))
        {
        wcl_status_close();
        strcpy(infobip_credit,"TIMEOUT");  //isteklo vrime
        PT_EXIT(pt);
        }
//�alje GET zahtjev
    gsmCreditState=0;
    ptr=0;
    tcp_write(wcl_status_pcb,CRBuf,strlen(CRBuf), 0);
    tcp_output(wcl_status_pcb);
    timer_set(&WCLCreditTimer,WCL_WAIT_TIME);
    PT_WAIT_UNTIL(pt,timer_expired(&WCLCreditTimer) || gsmCreditState==WCL_ANSWER);
    if(timer_expired((&WCLCreditTimer)))
        {
        wcl_status_close();
        strcpy(infobip_credit,"TIMEOUT");  //isteklo vrime
        PT_EXIT(pt);
        }
    wcl_status_close();
    PT_END(pt);
    }/***** ptWCLcredit() *****/






/*------------------------------------------------------------------------------**
** Funkcija aktivira mehanizam za slanje smsova 								**
**																				**
** smsN.......ako je varijabla 1, �alje se smsN 								**
**------------------------------------------------------------------------------*/
void send_infobip(UINT8 sms1,UINT8 sms2,UINT8 sms3,UINT8 sms4,UINT8 sms5)
    {
    sms.gsm1_enable=0;
    sms.gsm2_enable=0;
    sms.gsm3_enable=0;
    sms.gsm4_enable=0;
    sms.gsm5_enable=0;
    if(sms1)
        sms.gsm1_enable=1;
    if(sms2)
        sms.gsm2_enable=1;
    if(sms3)
        sms.gsm3_enable=1;
    if(sms4)
        sms.gsm4_enable=1;
    if(sms5)
        sms.gsm5_enable=1;
    sms.gsm5_enable=1;
    wcl_active=1;
    }//

/*------------------------------------------------------------------------------**
** Funkcija aktivira mehanizam za o�itavanje statusa							**
**																				**
**------------------------------------------------------------------------------*/
void get_credit(void)
    {
    wcl_credit_active=1;
    }//

void wcl_poll(void)
    {
    if(wcl_active)
        ptWCL(&pt_WCL);
    if(wcl_credit_active)
        ptWCLcredit(&pt_WCLcredit);
    }


