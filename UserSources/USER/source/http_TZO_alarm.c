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
#include <W25Q256.h>

#define SWAP_UINT32(w) (((w) & 0xff) << 8) | (((w) & 0xff00) >> 8)


#define TZO_WAIT_TIME	10000	//60000
#define TZO_WAITING	0
#define TZO_CONNECTED	1
#define TZO_READY	2
#define TZO_ACKED	3
#define TZO_ANSWER	4
#define TZO_ENCODE_BUF_LEN	1024


extern void temp_to_ascii(float rtemp,char *ascbuf);

char tzoState;  //,gsmCreditState;
struct timer TZOTimer;
struct pt pt_TZO;
struct ip4_addr tzo_ip;
unsigned int tzo_port=80;
//unsigned int wcl_port=8080;
err_t wclresult;
struct tcp_pcb *tzo_pcb,*tzo_status_pcb;
//char indata[1024];
char tzo_active=0;
UINT fwcl_rval;
char tzxBuf[1024] __attribute__ ((section(".ccm")));
char tzo_buf[TZO_ENCODE_BUF_LEN] __attribute__ ((section(".ccm")));
char tzo_retbuf[1024] __attribute__ ((section(".ccm")));
//char CRBuf[1024] __attribute__ ((section(".ccm")));
static long ptr;

int tzo_len,xmlpos;

//FIL wclfp;
//FRESULT wres;
UINT fwcl_rval;

//static err_t TZOConnected(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t TZORecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);


const char *sGisAlarmi[] = {"TEMPHIGH","TEMPLOW","TEMPNORMAL","DOOROPEN","DOORCLOSE"};

//*****************************************************************************
//
// wclient close cnnection
//
//*****************************************************************************
void tzo_close(void)
    {
    tcp_arg(tzo_pcb, NULL);
    tcp_sent(tzo_pcb, NULL);
    tcp_recv(tzo_pcb, NULL);
    tcp_close(tzo_pcb);
    tzo_active=0;
    }/**** tzo__close() ****/



//*****************************************************************************
//
// Error trap
//
//*****************************************************************************
void TZOerror(void *arg, err_t err)
    {
    }

static err_t TZOsent(void *arg, struct tcp_pcb *pcb,u16_t len)
    {
    LWIP_UNUSED_ARG(arg);
    return ERR_OK;
    }


//*****************************************************************************
//
// tcp_connect call-back
//
//*****************************************************************************
static err_t tzoConnected(void *arg, struct tcp_pcb *pcb, err_t err)
    {
    if (err == ERR_OK)
        {
        tcp_arg(pcb, NULL);
        tcp_sent(pcb, TZOsent);
        tcp_recv(pcb, TZORecv);
        tzoState=TZO_CONNECTED;
        }
    return err;
    }


//*****************************************************************************
//
// tcp_receive call-back
//
//*****************************************************************************
static err_t TZORecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
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
                tzo_retbuf[ptr+i]= pc[i];
            ptr+=p->len;
            lupAgain= p->next ? 1:0;	// test for null
            p= p->next;					// point to the next pbuf
            }
        while(lupAgain);
        tzoState=TZO_ANSWER;
        XMLParser("<status>",tzo_retbuf,p->len,infobip_status);
        XMLParser("<credits>",tzo_retbuf,p->len,infobip_credit);
        }
//if(logfile)
//	{
//	fwclresult=f_write(&wclfp,tzo_retbuf,strlen(tzo_retbuf),&fwcl_rval);
//	f_sync(&wclfp);
//	}
    return sock_err;
    }


/********************************************//**
 * \brief funkcija kreira string o nastanku alarma
 *
 * \param   sid     senzor ID
 * \param   atype   tip alarma
 * \param   avalue   numeriÄ�ka vrijednost alarma
 * \return NONE
 *
 ***********************************************/
void gisAlarm(uint8_t sid,uint8_t atype,float avalue)
    {
    char gisBuf[10];
    uint16_t intYear;
    XMLBuf[0]=0;
    getTime(&sat,&minuta,&sekunda);
    getDate (&godina,&mjesec,&datum,&dan);
    intYear=godina+2000;
    strcpy(XMLBuf,"gistzalarm=");
    OpenTag("GISTZALARM");
    strcat(XMLBuf,"\r\n");
    OpenTag("GENTIME");
    sprintf(gisBuf,"%d",intYear);
    strcat(XMLBuf,gisBuf);
    strcat(XMLBuf,"-");
    sprintf(gisBuf,"%d",mjesec);
    if(mjesec<10)
        strcat(XMLBuf,"0");
    strcat(XMLBuf,gisBuf);
    strcat(XMLBuf,"-");
    sprintf(gisBuf,"%d",datum);
    if(datum<10)
        strcat(XMLBuf,"0");
    strcat(XMLBuf,gisBuf);
    strcat(XMLBuf," ");


    sprintf(gisBuf,"%d",sat);
    if(sat<10)
        strcat(XMLBuf,"0");
    strcat(XMLBuf,gisBuf);
    strcat(XMLBuf,":");
    sprintf(gisBuf,"%d",minuta);
    if(minuta<10)
        strcat(XMLBuf,"0");
    strcat(XMLBuf,gisBuf);
    strcat(XMLBuf,":");
    sprintf(gisBuf,"%d",sekunda);
    if(sat<10)
        strcat(XMLBuf,"0");
    strcat(XMLBuf,gisBuf);

    CloseTag("GENTIME");
    strcat(XMLBuf,"\r\n");

    AddTag("CODE",sGisAlarmi[atype-1]);
    strcat(XMLBuf,"\r\n");
    sprintf((char*)tbuf,"%d",sid);
    AddTag("SENSORID",tbuf);
    strcat(XMLBuf,"\r\n");
    if(atype==DOOROPEN  || atype==DOORCLOSE)
        {
        if(atype==DOOROPEN)
            AddTag("VALUE","0");    //DOOROPEN
        else
            AddTag("VALUE","1");    //DOO
        }
    else
        {
        temp_to_ascii(avalue,tbuf);
        AddTag("VALUE",tbuf);
        strcat(XMLBuf,"\r\n");
        }
    strcat(XMLBuf,"\r\n");
    EEread(EE_OBJECT_NAME,tbuf,32);
    AddTag("WEBTHERMOID",tbuf);//--------<NAZIV>
    strcat(XMLBuf,"\r\n");
    CloseTag("GISTZALARM");
    strcat(XMLBuf,"\r\n");
    strcpy(tzo_buf,XMLBuf);
    tzo_len=strlen(tzo_buf);
//--------------------   //OVO je dodano da bi MIRU Å¡timao software, nema veze sa zdravom pameÄ‡u 01.07.2016

    strcat(tzo_buf,"           ");
//-------------------------------------------------------------------------

    tzo_active=1;

    }/***** gisAlarm() *****/





PT_THREAD(ptTZO(struct pt *pt))
    {
    uint32_t local_ip,u32tmp;
    PT_BEGIN(pt);
    PT_YIELD(pt);

    //tzo_len=CrateTZOString(tzo_buf);
    u32tmp=EEread_32(EE_ALARM_IP);
    local_ip = ((u32tmp>>24)&0xff) | // move byte 3 to byte 0
               ((u32tmp<<8)&0xff0000) | // move byte 1 to byte 2
               ((u32tmp>>8)&0xff00) | // move byte 2 to byte 1
               ((u32tmp<<24)&0xff000000); // byte 0 to byte 3
    ip4_addr_set_u32(&tzo_ip,local_ip);

    tzo_port=EEread_16(EE_ALARM_PORT);

    XMLBuf[0]=0;

    strcpy(XMLBuf,"POST /gistzalarm.php HTTP/1.1 \r\n");
    strcat(XMLBuf,"Host: ");
    LONGtoIP(rBuf,u32tmp);
    strcat(XMLBuf,rBuf);
    strcat(XMLBuf,"\r\n");
    strcat(XMLBuf,"Content-Type: application/x-www-form-urlencoded\r\n");
    strcat(XMLBuf,"Content-Length:");
    sprintf(rBuf,"%d",tzo_len);
    strcat(XMLBuf,rBuf);
    strcat(XMLBuf,"\r\n");
    strcat(XMLBuf,"Connection: close\r\n");
    strcat(XMLBuf,"\r\n");
    strcat(XMLBuf,tzo_buf);
    tzo_pcb=tcp_new();
    tcp_bind(tzo_pcb, IP_ADDR_ANY, 0);
    timer_set(&TZOTimer,TZO_WAIT_TIME);
    tzoState=0;
    ptr=0;
    wclresult= tcp_connect(tzo_pcb, &tzo_ip, tzo_port, tzoConnected);
    PT_WAIT_UNTIL(pt,timer_expired(&TZOTimer) || tzoState==TZO_CONNECTED);
    if(timer_expired((&TZOTimer)))
        {
        tzo_close();
        PT_EXIT(pt);
        }
//Å¡alje zaglavlje
    tzoState=0;
    timer_set(&TZOTimer,TZO_WAIT_TIME);
    ptr=0;
    tcp_output(tzo_pcb);
    tcp_write(tzo_pcb,XMLBuf,strlen(XMLBuf), 0);
    tcp_sent(tzo_pcb, tzo_close);
    tzo_active=0;
    PT_END(pt);
    }/***** ptTZO() *****/






/*------------------------------------------------------------------------------**
** Funkcija aktivira mehanizam za slanje alarma						**
**																				**
**------------------------------------------------------------------------------*/
void tzo_poll(void)
    {
    if(tzo_active)
        ptTZO(&pt_TZO);
    }


