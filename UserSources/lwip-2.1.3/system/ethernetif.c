/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
//#include "netif/ppp_oe.h"
//#include "err.h"
#include "ethernetif.h"
#include "encx24j600.h"
#include "main.h"
//#include "stm32_eth.h"
#include <string.h>

/* TCP and ARP timeouts */
volatile int tcp_end_time, arp_end_time;

/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 't'

#define  ETH_DMARxDesc_FrameLengthShift           16
#define  ETH_ERROR              ((u32)0)
#define  ETH_SUCCESS            ((u32)1)

extern struct MAC_ADDR
    {
	u08 v[6];
    } mac_addr;
#define ENC_TMP_LEN		1600
u08 enc_buf[ENC_TMP_LEN];

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif
    {
	struct eth_addr *ethaddr;
	/* Add whatever per-interface state that is needed here. */
	int unused;
    };

/* Forward declarations. */
err_t ethernetif_input(struct netif *netif);

#define ETH_RXBUFNB        2//4
#define ETH_TXBUFNB        2

uint8_t MACaddr[6];
u32 ETH_TxPkt_ChainMode(u16 FrameLength);

/**
 * Setting the MAC address.
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
void Set_MAC_Address(uint8_t* macadd)
    {
    MACaddr[0] = macadd[0];
    MACaddr[1] = macadd[1];
    MACaddr[2] = macadd[2];
    MACaddr[3] = macadd[3];
    MACaddr[4] = macadd[4];
    MACaddr[5] = macadd[5];

    }

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif)
    {
    enc424j600Init();


    netif->hwaddr[0] = mac_addr.v[0];
    netif->hwaddr[1] = mac_addr.v[1];
    netif->hwaddr[2] = mac_addr.v[2];
    netif->hwaddr[3] = mac_addr.v[3];
    netif->hwaddr[4] = mac_addr.v[4];
    netif->hwaddr[5] = mac_addr.v[5];

    /* maximum transfer unit */
    netif->mtu = 1500;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags =
	    NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    }

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t low_level_output(struct netif *netif, struct pbuf *p)
    {
    u16_t packet_len = p->tot_len;
    struct pbuf *q;
    int l = 0;
    u8 *buffer;
//u8 *buffer =  (u8 *)ETH_GetCurrentTxBuffer();

    for (q = p; q != NULL; q = q->next)
	{
	memcpy((u8_t*) &enc_buf[l], q->payload, q->len);
	l = l + q->len;
	}
// ovo bi trebalo rije�iti bez da se koristi buffer sa low level funkcijama
    enc424j600PacketSend(packet_len, &enc_buf[0]);
//  ETH_TxPkt_ChainMode(l);
    return ERR_OK;
    }/***** low_level_output() *****/

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
    {
    struct pbuf *p, *q;
    u16_t len;
    int l = 0;
//  FrameTypeDef frame;
    u8 *buffer;
    extern u16 enc424j600PacketReceive(u16, u08*);
    p = NULL;
//  frame = ETH_RxPkt_ChainMode();
    /* Obtain the size of the packet and put it into the "len"
     variable. */
    // len = frame.length;
    len = enc424j600PacketReceive(ENC_TMP_LEN, &enc_buf[0]);
    buffer = &enc_buf[0];

    /* We allocate a pbuf chain of pbufs from the pool. */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

    if (p != NULL)
	{
	for (q = p; q != NULL; q = q->next)
	    {
	    memcpy((u8_t*) q->payload, (u8_t*) &buffer[l], q->len);
	    l = l + q->len;
	    }
	}

    return p;
    }

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
err_t ethernetif_input(struct netif *netif)
    {
    err_t err;
    struct pbuf *p;

    /* move received packet into a new pbuf */
    p = low_level_input(netif);

    /* no packet could be read, silently ignore this */
    if (p == NULL)
	return ERR_MEM;

    err = netif->input(p, netif);
    if (err != ERR_OK)
	{
	LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
	pbuf_free(p);
	p = NULL;
	}

    return err;
    }

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif)
    {
    struct ethernetif *ethernetif;

    LWIP_ASSERT("netif != NULL", (netif != NULL));

    ethernetif = mem_malloc(sizeof(struct ethernetif));
    if (ethernetif == NULL)
	{
	LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
	return ERR_MEM;
	}

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    /*
     * Initialize the snmp variables and counters inside the struct netif.
     * The last argument should be replaced with your link speed, in units
     * of bits per second.
     */
    NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 100000000);

    netif->state = ethernetif;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
    netif->output = etharp_output;
    netif->linkoutput = low_level_output;

    ethernetif->ethaddr = (struct eth_addr *) &(netif->hwaddr[0]);

    /* initialize the hardware */
    low_level_init(netif);

    return ERR_OK;
    }

