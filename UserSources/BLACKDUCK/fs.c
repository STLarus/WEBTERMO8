/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "opt.h"
#include "def.h"
#include "fs.h"
#include "fsdata.h"
#include "strukture.h"
#include "httpd.h"
#include "lfs.h"
#include <string.h>
#include <stdint.h>
char file_name[32];

struct lfs_info fsinfo;


extern char XMLBuf[] __attribute__ ((section(".cmmram")));
;
/** Set this to 1 to include "fsdata_custom.c" instead of "fsdata.c" for the
 * file system (to prevent changing the file included in CVS) */
#ifndef HTTPD_USE_CUSTOM_FSDATA
#define HTTPD_USE_CUSTOM_FSDATA 0
#endif

#if HTTPD_USE_CUSTOM_FSDATA
#include "fsdata_custom.c"
#else /* HTTPD_USE_CUSTOM_FSDATA */
#include "fsdata.c"
#endif /* HTTPD_USE_CUSTOM_FSDATA */

/*-----------------------------------------------------------------------------------*/

#if LWIP_HTTPD_CUSTOM_FILES
int fs_open_custom(struct fs_file *file, const char *name);
void fs_close_custom(struct fs_file *file);
#if LWIP_HTTPD_FS_ASYNC_READ
u8_t fs_canread_custom(struct fs_file *file);
u8_t fs_wait_read_custom(struct fs_file *file, fs_wait_cb callback_fn, void *callback_arg);
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
#endif /* LWIP_HTTPD_CUSTOM_FILES */

#if LWIP_HTTPD_CUSTOM_FILES

void fs_close_custom(struct fs_file *file)
    {

    }

int fs_read_custom(struct fs_file *file, char *buffer, int count)
    {
    int read = 0;
    if (file->index < file->len)
	{
	read = file->len - file->index;
	if (read > count)
	    read = count;

	strcpy(file_name, file->fname);
	lfs_read(file_name, buffer, file->index, read);
	file->index += read;
	}
    else
	{
	read = FS_READ_EOF;
	}
    return read;
    }
#endif /* LWIP_HTTPD_CUSTOM_FILES */

/*-----------------------------------------------------------------------------------*/
err_t fs_open(struct fs_file *file, const char *name)
    {
    const struct fsdata_file *f;
    uint8_t k, j;
    char ekstenzija[10];

#ifdef HTTP_AUTHEN
    /* prvo se provjerava ekstenzija
     * ako je ekstenzija html a nije "login.html" ide se na provjeru authentikacije
     * ako je fajl "login.html", nema provjere authentikacije
     * za sve ostale ekstenzije nema provjere
     */
    if (0 != strcmp(name, "/login.html"))
	{
	//fajl nije "login.html" ide dalje i traži ekstenziju
	k = 0;
	j = 0;
	while (*(name + k) != '.')
	    {
	    k++;
	    }
	k++;	//preskace tocku
	while ((*(name + k) != 0))
	    {
	    if (*(name + k) == '(')
		break;
	    if (*(name + k) == '?')
		break;
	    ekstenzija[j++] = *(name + k);
	    k++;
	    }
	ekstenzija[j] = 0;
	if (strcmp(ekstenzija, "html") == 0)
	    {
	    // html fajl ide se na provjeru authentikacije
	    if (checkAuth(authen_uri) == 0) //ako authentikacija nije OK opet vraća login.html
		strcpy(name, "/login.html");
	    }
	}
#endif

    if ((file == NULL) || (name == NULL))
	{
	return ERR_ARG;
	}
    /*--------------------------------------------*/
    if (!strcmp((char*) name, "/disk0.xml"))
	{
	file->len = XMLDisk0(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}

    else if (!strcmp((char*) name, "/mreza.xml"))
	{
	file->len = XMLMreza(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/time.xml"))
	{
	file->len = XMLTime(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/sntp.xml"))
	{
	file->len = XMLSNTP(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}

    else if (!strcmp((char*) name, "/logfile.xml"))
	{
	file->len = XMLLogfile(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}

    else if (!strcmp((char*) name, "/sensor1.xml"))
	{
	file->len = XMLSensor1(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/sensor2.xml"))
	{
	file->len = XMLSensor2(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/sensor3.xml"))
	{
	file->len = XMLSensor3(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/sensor4.xml"))
	{
	file->len = XMLSensor4(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/sensor5.xml"))
	{
	file->len = XMLSensor5(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/sensor6.xml"))
	{
	file->len = XMLSensor6(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/sensor7.xml"))
	{
	file->len = XMLSensor7(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/sensor8.xml"))
	{
	file->len = XMLSensor8(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/home.xml"))
	{
	file->len = XMLHome(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/code.xml"))
	{
	file->len = XMLCode(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}

    else if (!strcmp((char*) name, "/online.xml"))
	{
	file->len = XMLOnline(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/sms.xml"))
	{
	file->len = XMLSms(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/credit.xml"))
	{
	file->len = XMLCredit(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/startcredit.xml"))
	{
	file->len = XMLStartCredit(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}

    else if (!strcmp((char*) name, "/smsstatus.xml"))
	{
	file->len = XMLSmsstatus(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}

    else if (!strcmp((char*) name, "/sample.xml"))
	{
	file->len = XMLSample(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/haccp.xml"))
	{
	file->len = XMLHaccp(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/ok.xml"))
	{
	file->len = XMLOk(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/ulazi.xml"))
	{
	file->len = XMLUlazi(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/input.xml"))
	{
	file->len = XMLInput(file->data);
	file->data = XMLBuf;
	file->index = file->len;
	file->pextension = NULL;
	return ERR_OK;
	}
    else if (!strcmp((char*) name, "/wapp.xml"))
    	{
    	file->len = XMLWapp(file->data);
    	file->data = XMLBuf;
    	file->index = file->len;
    	file->pextension = NULL;
    	return ERR_OK;
    	}
    else if (!strcmp((char*) name, "/mqtt.xml"))
       	{
       	file->len = XMLMQTT(file->data);
       	file->data = XMLBuf;
       	file->index = file->len;
       	file->pextension = NULL;
       	return ERR_OK;
       	}
    else if (!strcmp((char*) name, "/onlineinput.xml"))
       	{
       	file->len = XMLOnlineInput(file->data);
       	file->data = XMLBuf;
       	file->index = file->len;
       	file->pextension = NULL;
       	return ERR_OK;
       	}
    /*--------------------------------------------*/
    //--------XML tagovi-------------------

#if LWIP_HTTPD_CUSTOM_FILES

    strcpy(file->fname, name);

    strcpy(file_name, file->fname);
    int err = lfs_file_opencfg(&wtlfs, &http_file, &file_name[1], LFS_O_RDONLY,&http_cfg);

    if (0 > err)
	file->is_custom_file = 0;
    else
	{
	lfs_stat(&wtlfs, file->fname, &fsinfo);
	file->is_custom_file = 1;
	file->data = "";
	file->len = fsinfo.size;
	err = lfs_file_close(&wtlfs, &http_file);
	file->index = 0;	//stbuf.size;
	file->pextension = NULL;

	return ERR_OK;
	}
    //file->is_custom_file = 0;
#endif /* LWIP_HTTPD_CUSTOM_FILES */

    for (f = FS_ROOT; f != NULL; f = f->next)
	{
	if (!strcmp(name, (char *) f->name))
	    {
	    file->data = (const char *) f->data;
	    file->len = f->len;
	    file->index = f->len;
	    file->pextension = NULL;
	    file->http_header_included = f->http_header_included;
#if HTTPD_PRECALCULATED_CHECKSUM
	    file->chksum_count = f->chksum_count;
	    file->chksum = f->chksum;
#endif /* HTTPD_PRECALCULATED_CHECKSUM */
#if LWIP_HTTPD_FILE_STATE
	    file->state = fs_state_init(file, name);
#endif /* #if LWIP_HTTPD_FILE_STATE */

	    return ERR_OK;
	    }
	}
    /* file not found */
    return ERR_VAL;
    }

/*-----------------------------------------------------------------------------------*/
void fs_close(struct fs_file *file)
    {
#if LWIP_HTTPD_CUSTOM_FILES
    if (file->is_custom_file)
	{
	fs_close_custom(file);
	}
#endif /* LWIP_HTTPD_CUSTOM_FILES */
#if LWIP_HTTPD_FILE_STATE
    fs_state_free(file, file->state);
#endif /* #if LWIP_HTTPD_FILE_STATE */
    //    LWIP_UNUSED_ARG(file);
    }
/*-----------------------------------------------------------------------------------*/
#if LWIP_HTTPD_DYNAMIC_FILE_READ
#if LWIP_HTTPD_FS_ASYNC_READ
int
fs_read_async(struct fs_file *file, char *buffer, int count, fs_wait_cb callback_fn, void *callback_arg)
#else /* LWIP_HTTPD_FS_ASYNC_READ */
int fs_read(struct fs_file *file, char *buffer, int count)
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
    {

    int read;

    if (file->index == file->len)
	{
	return FS_READ_EOF;
	}
#if LWIP_HTTPD_FS_ASYNC_READ
#if LWIP_HTTPD_CUSTOM_FILES
    if (!fs_canread_custom(file))
	{
	if (fs_wait_read_custom(file, callback_fn, callback_arg))
	    {
	    return FS_READ_DELAYED;
	    }
	}
#else /* LWIP_HTTPD_CUSTOM_FILES */
    LWIP_UNUSED_ARG(callback_fn);
    LWIP_UNUSED_ARG(callback_arg);
#endif /* LWIP_HTTPD_CUSTOM_FILES */
#endif /* LWIP_HTTPD_FS_ASYNC_READ */

    if (file->is_custom_file)
	{
	return fs_read_custom(file, buffer, count);

	}
    else
	{

	read = file->len - file->index;
	if (read > count)
	    {
	    read = count;
	    }

	MEMCPY(buffer, (file->data + file->index), read);
	file->index += read;
	}
    return (read);
    }
#endif /* LWIP_HTTPD_DYNAMIC_FILE_READ */
/*-----------------------------------------------------------------------------------*/
#if LWIP_HTTPD_FS_ASYNC_READ
int
fs_is_file_ready(struct fs_file *file, fs_wait_cb callback_fn, void *callback_arg)
    {
    if (file != NULL)
	{
#if LWIP_HTTPD_FS_ASYNC_READ
#if LWIP_HTTPD_CUSTOM_FILES
	if (!fs_canread_custom(file))
	    {
	    if (fs_wait_read_custom(file, callback_fn, callback_arg))
		{
		return 0;
		}
	    }
#else /* LWIP_HTTPD_CUSTOM_FILES */
	LWIP_UNUSED_ARG(callback_fn);
	LWIP_UNUSED_ARG(callback_arg);
#endif /* LWIP_HTTPD_CUSTOM_FILES */
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
	}
    return 1;
    }
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
/*-----------------------------------------------------------------------------------*/
int fs_bytes_left(struct fs_file *file)
    {
    return file->len - file->index;
    }
