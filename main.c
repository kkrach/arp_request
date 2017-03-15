/** Sends ARP request for given IP address
 *
 * Author: Chaitanya bhargav M
 * Source: http://linux-spawn.blogspot.de/2009/10/sending-arp-request-in-linux-using-c.html
 */

#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "net/if.h"
#include "stdlib.h"
#include "arpa/inet.h"
#include "sys/ioctl.h"
#include "netpacket/packet.h"
#include "net/ethernet.h"
#include "net/if_arp.h"





struct arp_packet
{
	struct ether_header ether;      // Ethernet Header
	struct arphdr arp;              // ARP Header

	uint8_t  sender_mac[ETH_ALEN];
	uint32_t sender_ip;
	uint8_t  target_mac[ETH_ALEN];
	uint32_t target_ip;

	uint8_t  padding[18];           // Paddign
} __attribute__ ((__packed__));



int main(int argc, char *argv[])
{
	int arp_fd,if_fd,retVal;
	struct sockaddr_in *sin;
	struct sockaddr_ll sa;
	struct ifreq ifr;
	struct arp_packet pkt;
	uint32_t ipAddr;
	
	if( argc != 3 )
	{
		printf("Usage: sudo %s INTERFACE IP\n", argv[0]);
		printf("         INTERFACE    network interface, e.g. eth0\n");
		printf("         IP           IP to resolve, e.g. 192.168.10.7\n");
		exit(1);
	}
	else if( getuid() && geteuid())
	{
		printf("Oops!\nDude you need SuperUser rights!\n");
		exit(1);
	}
	
	/*=============================START of IP, MAC ADDRESS ACCESS========================*/
	
	/* Open socket for accessing the IPv4 address of specified Interface */
	if_fd = socket(AF_INET, SOCK_STREAM, 0);
	if( if_fd < 0 )
	{
		perror("IF Socket");
		exit(1);
	}
	
	/* provide interface name to ifreq structure */
	memcpy(ifr.ifr_name, argv[1], IF_NAMESIZE);
	/* IOCTL to get ip address */
	retVal = ioctl(if_fd, SIOCGIFADDR, &ifr, sizeof(ifr));
	if( retVal < 0 ) {
		perror("IOCTL");
		exit(1);
	}
	
	/* Simple typecasting for easy access to ip address */
	sin = (struct sockaddr_in *)&ifr.ifr_addr;
	ipAddr = ntohl(sin->sin_addr.s_addr);

	
	retVal = ioctl(if_fd, SIOCGIFHWADDR, &ifr, sizeof(ifr));
	if( retVal < 0 ) {
		perror("IOCTL");
		exit(1);
	}
	
	/*-----------------------------END of IP,MAC ADDRESS ACCESS------------------------*/
	
	/*=============================Start of ARP request sending====================*/
	// Socket to send ARP packet
	arp_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if( arp_fd == -1 ) {
		perror("ARP Socket");
		exit(1);
	}
	//====================== Formulate the ARP Packet ============================

	memset(pkt.ether.ether_dhost, 0xFF, sizeof(pkt.ether.ether_dhost));
	memcpy(pkt.ether.ether_shost, ifr.ifr_hwaddr.sa_data, sizeof(pkt.ether.ether_dhost));
	pkt.ether.ether_type = htons(ETHERTYPE_ARP);

	pkt.arp.ar_hrd = htons(ARPHRD_ETHER);
	pkt.arp.ar_pro = htons(ETHERTYPE_IP);
	pkt.arp.ar_hln = ETHER_ADDR_LEN;
	pkt.arp.ar_pln = sizeof(pkt.sender_ip);
	pkt.arp.ar_op = htons(ARPOP_REQUEST);

	memcpy(pkt.sender_mac, ifr.ifr_hwaddr.sa_data, sizeof(pkt.sender_mac));
	pkt.sender_ip = htonl(ipAddr);
	memset(pkt.target_mac, 0 , sizeof(pkt.target_mac));
	pkt.target_ip = inet_addr(argv[2]);

	memset(pkt.padding, 0 , sizeof(pkt.padding));
	
	
	// For sending the packet We need it!
	retVal = ioctl(if_fd, SIOCGIFINDEX, &ifr, sizeof(ifr));
	if( retVal < 0 ) {
		perror("IOCTL");
		exit(1);
	}
	sa.sll_family = AF_PACKET;
	sa.sll_ifindex = ifr.ifr_ifindex;
	sa.sll_protocol = htons(ETH_P_ARP);
	sa.sll_halen = 0;
	
	/* Send it! */
	retVal = sendto(arp_fd, &pkt, sizeof(pkt), 0,(struct sockaddr *)&sa, sizeof(sa));
	if( retVal < 0 ) {
		perror("sendto");
		exit(1);
	}

	close(arp_fd);
	close(if_fd);

	return 0;
}

