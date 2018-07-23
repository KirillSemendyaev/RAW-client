#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>


int main(int argc, char **argv)
{
	if (argc != 4) {
		printf("Usage: ./RAW_CLIENT [dest_ip] [port] [source_ip]\n");
		return -2;
	}

	char buf[512] = {0};
	int socket_fd, ret, on = 1;
	size_t len;
	struct sockaddr_ll target;
	struct udphdr udph, *udp_ptr;
	struct ethhdr ethh;
	struct iphdr iph, *ip_ptr;
	socklen_t target_size;

	socket_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (socket_fd == -1) {
		perror("socket");
		return -1;
	}

	memset(&ethh, 0, sizeof(ethh));
	memset(&target, 0, sizeof(target));
	memset(&iph, 0, sizeof(iph));
	memset(&udph, 0, sizeof(udph));
	target.sll_family = AF_PACKET;
	target.sll_ifindex = if_nametoindex("enp2s0");
	target.sll_halen = 6;
	target_size = sizeof(target);
	len = sizeof(buf);

	ethh.h_dest[0] = 0xd8;
	ethh.h_dest[1] = 0xcb;
	ethh.h_dest[2] = 0x8a;
	ethh.h_dest[3] = 0xf4;
	ethh.h_dest[4] = 0x72;
	ethh.h_dest[5] = 0xb1;
	ethh.h_source[0] = 0xe8;
	ethh.h_source[1] = 0x03;
	ethh.h_source[2] = 0x9a;
	ethh.h_source[3] = 0xb6;
	ethh.h_source[4] = 0xf2;
	ethh.h_source[5] = 0xde;
	ethh.h_proto = htons(ETH_P_IP);


	udph.check = 0;
	udph.dest = htons(atoi(argv[2]));
	udph.source = htons(60000);
	udph.len = htons(len - sizeof(iph) - sizeof(ethh));

	iph.version = 4;
	iph.ihl = sizeof(iph) / 4;
	iph.tos = 0;
	iph.tot_len = htons(len - sizeof(ethh));
	iph.id = htons(8008);
	iph.frag_off = 0;
	iph.ttl = 255;
	iph.protocol = IPPROTO_UDP;
	iph.saddr = inet_addr(argv[3]);
	iph.daddr = inet_addr(argv[1]);
	iph.check = 0;

	memcpy(buf, &ethh, sizeof(ethh));
	memcpy(buf + sizeof(ethh), &iph, sizeof(iph));
	memcpy(buf + sizeof(iph) + sizeof(ethh), &udph, sizeof(udph));
	sprintf(buf + sizeof(udph) + sizeof(iph) + sizeof(ethh), "Hello!");
	printf("%s\n", buf + sizeof(udph) + sizeof(iph) + sizeof(ethh));

	ret = sendto(socket_fd, buf, len, 0, (struct sockaddr*)&target, target_size);
	if (ret == -1) {
		perror("send");
		return -3;
	}

	do {
		ret = recvfrom(socket_fd, buf, len, 0, (struct sockaddr *) &target, &target_size);
		udp_ptr = buf + sizeof(iph) + sizeof(ethh);
		ip_ptr = buf + sizeof(ethh);
		printf("%d: %s\n", ntohs(udp_ptr->dest), buf + sizeof(udph) + sizeof(iph) + sizeof(ethh));
	} while (ntohs(udp_ptr->dest) != 60000);
	printf("IN: %s\n", buf + sizeof(udph) + sizeof(iph) + sizeof(ethh));
	close(socket_fd);
	return 0;
}


