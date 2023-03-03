#ifndef IP_ARP_UDP_TCP_H
#define IP_ARP_UDP_TCP_H


// you must call this function once before you use any of the other functions:
extern void init_ip_arp_udp(unsigned char *mymac,unsigned char *myip);
extern unsigned char eth_type_is_arp_and_my_ip(unsigned char *buf,unsigned int len);
extern unsigned char eth_type_is_ip_and_my_ip(unsigned char *buf,unsigned int len);
extern void make_arp_answer_from_request(unsigned char *buf);
extern void make_echo_reply_from_request(unsigned char *buf,unsigned int len);
extern void make_udp_reply_from_request(unsigned char *buf,char *data,unsigned char datalen,unsigned int  port);





#endif /* IP_ARP_UDP_TCP_H */
//@}
