#include "stm32f4xx.h"
#include "strukture.h"
#include "define.h"
#include "pt.h"
#include "timer.h"
#include "datatypes.h"
#include "system.h"
//#include "tftpserver.h"
//#include "tftputils.h"
#include "udp.h"
#include "drajveri.h"
#include "logger.h"
#include "lwip/ip.h"
#include <stdio.h>
#include <string.h>

struct timer BackupScanTimer;
struct pt pt_SendBackupFile;

struct udp_pcb *backup_pcb;
struct ip4_addr backup_addr;
UINT16 BACKUP_PORT;
UINT16 backup_setup_min,backup_old_min;

char backup_status;
char backname[10],tftp_server_path[240];
char tftp_data[512+4];
UINT16 tftp_data_len;
UINT16 tftp_block;/* next block number */
UINT16 tftp_tot_bytes;/* total number of bytes transferred */


#define TFTP_RRQ   1
#define TFTP_WRQ   2
#define TFTP_DATA  3
#define TFTP_ACK   4
#define TFTP_ERROR 5


/********************************************************************************************************/
UINT8 TestBackupTime(void)
/********************************************************************************************************/
{
UINT16 backup_min;
backup_setup_min=(UINT8)(*(__IO uint32_t *) (BKPSRAM_BASE + 0x05))*256;
backup_min=(UINT8)(*(__IO uint32_t *) (BKPSRAM_BASE + 0x06));
backup_setup_min+=backup_min;
getTime(&sat,&minuta,&sekunda);
backup_min=sat*60+minuta;
if(backup_min==backup_setup_min)
	{
	if(backup_old_min!=backup_min)
		{
		backup_old_min=backup_min;
		return 1;
		}
	}
backup_old_min=backup_min;
return 0;
}/***** TestBackupTime() *****/


/********************************************************************************************************/
void tftp_backup_send(u8_t *data, u16_t len)
/********************************************************************************************************/
{
/*struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT,len, PBUF_RAM);
if (backup_pcb->remote_port != (uint16_t)0)
	{
	memcpy(p->payload, data, len);
	udp_send(backup_pcb, p);
    }
    */
}/**** tftp_backup_send() *****/


/********************************************************************************************************/
void tftp_backup_receive(void *arg, struct udp_pcb *upcb,struct pbuf *p, struct ip_addr *addr, u16_t port)
/********************************************************************************************************/
{
//tftp_process_read(backup_pcb,&backup_addr,backup_pcb->remote_port,backname);
pbuf_free(p);                                   /* don't leak the pbuf! */
}/**** tftp_backup_receive() *****/



/**********************************************/
void tftp_backup_init(void)
/**********************************************/
{
err_t err;
//READ_FLASH(0xF00,rBuf,256);
BACKUP_PORT =rBuf[0x04]*0x100+rBuf[0x05];
IP4_ADDR(&backup_addr, rBuf[0x00],rBuf[0x01],rBuf[0x02],rBuf[0x03]);	//adresa TFTP servera

/* create a new UDP PCB structure  */
backup_pcb = udp_new();
if (!backup_pcb)
	return;/* Error creating PCB. Out of Memory  */
err = udp_bind(backup_pcb, IP_ADDR_ANY, 777);
if (err != ERR_OK)
	return;/* Unable to bind to port  */
udp_connect(backup_pcb ,&backup_addr, BACKUP_PORT);/* TFTP client start  */
udp_recv(backup_pcb,tftp_backup_receive,NULL);
}/**** tftp_backup_init() *****/


PT_THREAD(ptSendBackupFile(struct pt *pt))
{
UINT16 str_path_len;
UINT8 fnamelen;
char tstr[16];
PT_BEGIN(pt);
PT_YIELD(pt);
tftp_backup_init();
getDate (&godina,&mjesec,&datum,&dan);
getTime(&sat,&minuta,&sekunda);
CreateDatName(&backname[0],godina,mjesec);	//ime fajla koji se �alje
sprintf(tstr,"%04d",godina+2000);
strcpy(tftp_server_path,tstr);
sprintf(tstr,"%02d",mjesec);
strcat(tftp_server_path,tstr);
sprintf(tstr,"%02d",datum);
strcat(tftp_server_path,tstr);
sprintf(tstr,"%02d",sat);
strcat(tftp_server_path,tstr);
sprintf(tstr,"%02d",minuta);
strcat(tftp_server_path,tstr);
sprintf(tstr,"%02d",sekunda);
strcat(tftp_server_path,tstr);	//ime fajla bez putanje
strcat(tftp_server_path,".csv");
//tftp_set_opcode(tftp_data,TFTP_WRQ);
//kreira string za prvu komandu
strcpy(&tftp_data[2],tftp_server_path);
fnamelen=strlen(tftp_server_path)+2;
tftp_data[fnamelen++]=0;
memcpy(&tftp_data[fnamelen],"octet",5);
fnamelen+=5;
tftp_data[fnamelen++]=0;
tftp_backup_send(tftp_data,fnamelen);	//poslana prva komanda
backup_scan_flag=0;
PT_END(pt);
}/***** ptSendBackupFile() *****/



UINT16 CheckBackup(void)
{
backup_status=(*(__IO uint32_t *) (BKPSRAM_BASE + 0x07));
if(backup_status)
	{
	if(timer_expired(&BackupScanTimer))
		{
		timer_set(&BackupScanTimer,1000);
		if(TestBackupTime())
			backup_scan_flag=1;
		}
	}
if(backup_scan_flag==1)
	  ptSendBackupFile(&pt_SendBackupFile);
return 0;
}/***** CheckBackup() *****/

