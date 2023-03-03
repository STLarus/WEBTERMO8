/*
 * WhatsApp.c
 *
 *  Created on: 26. velj 2023.
 *      Author: teovu
 */
#include "http_client.h"
#include "define.h"

static err_t wappHeadresDone(httpc_state_t*, void*, struct pbuf*, u16_t, u32_t);
static void wappResult(void*, httpc_result_t, u32_t, u32_t, err_t);

extern uint8_t wAppStatus, wAppflag;
//------------  https://www.unshiu.com/posts/pico-http-client-part-i-simple-client/

httpc_connection_t client_settings =
    {
    .use_proxy = 0, .headers_done_fn = wappHeadresDone, .result_fn = wappResult,
    };

//httpc_connection_t client_setting;
//httpc_connection_t *conn_settings;

httpc_state_t *connection = NULL;
char wappNum[34];
char wappApiKey[34];
char callmebot_url[] = "/whatsapp.php?phone=%s&text=%s&apikey=%s";
char URL[200];

char* url_encode(const char *str)
    {
    static const char *hex = "0123456789abcdef";
    static char encoded[1024];
    char *p = encoded;
    while (*str)
	{
	if (isalnum(*str) || *str == '-' || *str == '_' || *str == '.'
		|| *str == '~')
	    {
	    *p++ = *str;
	    }
	else
	    {
	    *p++ = '%';
	    *p++ = hex[*str >> 4];
	    *p++ = hex[*str & 15];
	    }
	str++;
	}
    *p = '\0';
    return encoded;
    }

static err_t wappHeadresDone(httpc_state_t *connection, void *arg,
	struct pbuf *hdr, u16_t hdr_len, u32_t content_len)
    {
    printf("in headers_done_fn\n");
    return ERR_OK;
    }

static void wappResult(void *arg, httpc_result_t httpc_result,
	u32_t rx_content_len, u32_t srv_res, err_t err)
    {
    printf(">>> result_fn >>>\n");
    printf("httpc_result: %s\n",
	    httpc_result == HTTPC_RESULT_OK ? "HTTPC_RESULT_OK" :
	    httpc_result == HTTPC_RESULT_ERR_UNKNOWN ?
		    "HTTPC_RESULT_ERR_UNKNOWN" :
	    httpc_result == HTTPC_RESULT_ERR_CONNECT ?
		    "HTTPC_RESULT_ERR_CONNECT" :
	    httpc_result == HTTPC_RESULT_ERR_HOSTNAME ?
		    "HTTPC_RESULT_ERR_HOSTNAME" :
	    httpc_result == HTTPC_RESULT_ERR_CLOSED ?
		    "HTTPC_RESULT_ERR_CLOSED" :
	    httpc_result == HTTPC_RESULT_ERR_TIMEOUT ?
		    "HTTPC_RESULT_ERR_TIMEOUT" :
	    httpc_result == HTTPC_RESULT_ERR_SVR_RESP ?
		    "HTTPC_RESULT_ERR_SVR_RESP" :
	    httpc_result == HTTPC_RESULT_ERR_MEM ? "HTTPC_RESULT_ERR_MEM" :
	    httpc_result == HTTPC_RESULT_LOCAL_ABORT ?
		    "HTTPC_RESULT_LOCAL_ABORT" :
	    httpc_result == HTTPC_RESULT_ERR_CONTENT_LEN ?
		    "HTTPC_RESULT_ERR_CONTENT_LEN" : "*UNKNOWN*");
    printf("received %ld bytes\n", rx_content_len);
    printf("server response: %ld\n", srv_res);
    printf("err: %d\n", err);
    printf("<<< result_fn <<<\n");
    }

static err_t wappReceive(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
	err_t err)
    {
    printf(">>> recv_fn >>>\n");
    if (p == NULL)
	{
	printf("p is NULL\n");
	}
    else
	{
	printf("p: %p\n", p);
	printf("next: %p\n", p->next);
	printf("payload: %p\n", p->payload);
	printf("len: %d\n", p->len);
	}
    printf("<<< recv_fn <<<\n");

    wAppStatus = WAPP_FREE;
    return ERR_OK;
    }

void send_whatsapp_message(uint8_t snum, char *wappMessage)
    {
    snum--;
    EEread(EE_WAPP_START + (snum * 0x80) + (EE_WAPP1_NUM-EE_WAPP_START), &wappNum[0], 32); //broj telefona
    EEread(EE_WAPP_START + (snum * 0x80) + (EE_WAPP1_KEY-EE_WAPP_START), &wappApiKey[0], 32); //api key

    sprintf(URL, callmebot_url, wappNum, url_encode(wappMessage), wappApiKey);

    httpc_get_file_dns("api.callmebot.com", 80, URL, &client_settings,
	    wappReceive,
	    NULL, &connection);
    }

