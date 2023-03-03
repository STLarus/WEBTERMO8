#include "lwip/apps/mqtt.h"
#include "netif.h"
#include "mqtt_example.h"
#include "strukture.h"
#include "timer.h"
#include "pt.h"

extern void temp_to_ascii(float, char*);
extern struct netif netif;

struct pt pt_MQTT;
struct timer mqttTimer;

PT_THREAD( ptMQTT(struct pt*));

#if LWIP_TCP

/** Define this to a compile-time IP address initialization
 * to connect anything else than IPv4 loopback
 */
//192.168.0.100
#define MQTTSERVER_IP	((u32_t)0xC0A80064UL)
//192.168.1.100
//#define MQTTSERVER_IP	((u32_t)0xC0A80164UL)
//192.168.1.103
//#define MQTTSERVER_IP	((u32_t)0xC0A80167UL)
//5.196.95.208	test.mosquitto.org
//#define MQTTSERVER_IP	((u32_t)0x05C45FD0UL)
// HIVEmq  broker. hivemq.com   18.158.198.79
//#define MQTTSERVER_IP	((u32_t)0x129EC64FUL)

#define  LWIP_MQTT_EXAMPLE_IPADDR_INIT= IPADDR4_INIT(PP_HTONL(MQTTSERVER_IP))

#ifndef LWIP_MQTT_EXAMPLE_IPADDR_INIT
#if LWIP_IPV4
#define LWIP_MQTT_EXAMPLE_IPADDR_INIT = IPADDR4_INIT(PP_HTONL(IPADDR_LOOPBACK))
#else
#define LWIP_MQTT_EXAMPLE_IPADDR_INIT
#endif
#endif

static ip_addr_t mqtt_ip LWIP_MQTT_EXAMPLE_IPADDR_INIT
;
static mqtt_client_t *mqtt_client;

//http://mqtt-explorer.com/
//https://delightnet.nl/index.php/mqtt/12-mqtt-broker-installation
//https://mqtt.org/software/
//https://mcuoneclipse.com/2017/04/09/mqtt-with-lwip-and-nxp-frdm-k64f-board/
//  T L S   https://mcuoneclipse.com/2017/04/17/tutorial-secure-tls-communication-with-mqtt-using-mbedtls-on-top-of-lwip/
//  http://www.steves-internet-guide.com/mossquitto-conf-file/


//static const struct mqtt_connect_client_info_t mqtt_client_info =
//    {
//    "test", "admin", /* user */
//    "hivemq", /* pass */
//    100, /* keep alive */
//    NULL, /* will_topic */
//    NULL, /* will_msg */
//    0, /* will_qos */
//    0
//    /* will_retain */
//#if LWIP_ALTCP && LWIP_ALTCP_TLS
//  , NULL
//#endif
//	};

static const struct mqtt_connect_client_info_t mqtt_client_info =
    {
    "test", "larus", /* user */
    "peljescanka", /* pass */
    100, /* keep alive */
    NULL, /* will_topic */
    NULL, /* will_msg */
    0, /* will_qos */
    0
    /* will_retain */
#if LWIP_ALTCP && LWIP_ALTCP_TLS
  , NULL
#endif
	};

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len,
	u8_t flags)
    {
    const struct mqtt_connect_client_info_t *client_info =
	    (const struct mqtt_connect_client_info_t*) arg;
    LWIP_UNUSED_ARG(data);

    LWIP_PLATFORM_DIAG(
	    ("MQTT client \"%s\" data cb: len %d, flags %d\n", client_info->client_id, (int)len, (int)flags));
    }

static void mqtt_incoming_publish_cb(void *arg, const char *topic,
	u32_t tot_len)
    {
    const struct mqtt_connect_client_info_t *client_info =
	    (const struct mqtt_connect_client_info_t*) arg;

    LWIP_PLATFORM_DIAG(
	    ("MQTT client \"%s\" publish cb: topic %s, len %d\n", client_info->client_id, topic, (int)tot_len));
    }

static void mqtt_request_cb(void *arg, err_t err)
    {
    const struct mqtt_connect_client_info_t *client_info =
	    (const struct mqtt_connect_client_info_t*) arg;

    LWIP_PLATFORM_DIAG(
	    ("MQTT client \"%s\" request cb: err %d\n", client_info->client_id, (int)err));
    }

static void mqtt_connection_cb(mqtt_client_t *client, void *arg,
	mqtt_connection_status_t status)
    {
    const struct mqtt_connect_client_info_t *client_info =
	    (const struct mqtt_connect_client_info_t*) arg;
    LWIP_UNUSED_ARG(client);

    LWIP_PLATFORM_DIAG(
	    ("MQTT client \"%s\" connection cb: status %d\n", client_info->client_id, (int)status));

    if (status == MQTT_CONNECT_ACCEPTED)
	{
	mqtt_sub_unsub(client, "topic_qos1", 1, mqtt_request_cb,
		LWIP_CONST_CAST(void*, client_info), 1);
	mqtt_sub_unsub(client, "topic_qos0", 0, mqtt_request_cb,
		LWIP_CONST_CAST(void*, client_info), 1);
	}
    }

#endif /* LWIP_TCP */

void mqtt_example_init(void)
    {
    mqtt_client = mqtt_client_new();

    mqtt_set_inpub_callback(mqtt_client, mqtt_incoming_publish_cb,
	    mqtt_incoming_data_cb, LWIP_CONST_CAST(void*, &mqtt_client_info));

    mqtt_client_connect(mqtt_client, &mqtt_ip, MQTT_PORT, mqtt_connection_cb,
	    LWIP_CONST_CAST(void*, &mqtt_client_info), &mqtt_client_info);
    }

static void mqtt_pub_request_cb(void *arg, err_t result)
    {
    if (result != ERR_OK)
	printf("Publish err: %d\n", result);

    }

void MqttPublish(uint8_t senzor)
    {
    //const char *pub_payload="abcd";
    void *arg;
    err_t err;
    uint8_t qos = 0;
    uint8_t retain = 0;
    char mqttBuf[30];
    if (senzor == 1)
	{
	temp_to_ascii(temp[0], mqttBuf);
	err = mqtt_publish(mqtt_client, "webtermo/sen1/temp", mqttBuf,
		strlen(mqttBuf), qos, retain, mqtt_pub_request_cb, arg);
	}
    else if (senzor == 2)
	{
	temp_to_ascii(temp[1], mqttBuf);
	err = mqtt_publish(mqtt_client, "webtermo/sen2/temp", mqttBuf,
		strlen(mqttBuf), qos, retain, mqtt_pub_request_cb, arg);
	}

    if (err != ERR_OK)
	printf("Publish err: %d\n", err);
    }

void mqtt_connect()
    {
    uint8_t netif_state;
    netif_state = netif.flags & NETIF_FLAG_LINK_UP;
    if (netif_state)
	{
	if (!mqtt_client_is_connected(mqtt_client))
	    { //nema konekcije , konektiraj se
	    mqtt_client = mqtt_client_new();

	    mqtt_set_inpub_callback(mqtt_client, mqtt_incoming_publish_cb,
		    mqtt_incoming_data_cb,
		    LWIP_CONST_CAST(void*, &mqtt_client_info));

	    mqtt_client_connect(mqtt_client, &mqtt_ip, MQTT_PORT,
		    mqtt_connection_cb,
		    LWIP_CONST_CAST(void*, &mqtt_client_info),
		    &mqtt_client_info);
	    }
	else
	    {
	    MqttPublish(1);
	    //MqttPublish(2);
	    }
	}

    else
	{
	mqtt_disconnect(mqtt_client);
	}

    }

void mqtt_scan(void)
    {
    ptMQTT(&pt_MQTT);
    }

PT_THREAD(ptMQTT(struct pt *pt))
    {

    PT_BEGIN(pt)
    ;

    //PT_YIELD(pt);
    timer_set(&mqttTimer, 30000);
    PT_WAIT_UNTIL(pt, timer_expired(&mqttTimer));
    mqtt_connect();
    MqttPublish(1);
    timer_set(&mqttTimer, 20000);
    PT_WAIT_UNTIL(pt, timer_expired(&mqttTimer));
    MqttPublish(2);
    timer_set(&mqttTimer, 20000);
    PT_WAIT_UNTIL(pt, timer_expired(&mqttTimer));

PT_END(pt);
}

