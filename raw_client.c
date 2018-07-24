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

__u16 checksum(__u16 *buf, __u32 size)
{
	if ((buf == NULL) || (size < 1)) {
		return 0;
	}
	__u32 sum = 0, ret;

	for (int i = 0; i < (size / 2); ++i) {
		sum += buf[i];
	}
	ret = (sum >> 16) + (sum & 0xffff);
	return ~ret;
}


int main(int argc, char **argv)
{
	if (argc != 4) {
		printf("Usage: ./RAW_CLIENT [dest_ip] [port] [source_ip]\n");
		return -2;
	}

	char buf[512] = {0};
	int socket_fd, ret;
	size_t len;
	struct sockaddr_ll target;
	struct udphdr *udph;
	struct ethhdr *ethh;
	struct iphdr *iph;
	socklen_t target_size;

	socket_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (socket_fd == -1) {
		perror("socket");
		return -1;
	}

	memset(&target, 0, sizeof(target));
	target.sll_family = AF_PACKET;
	target.sll_ifindex = if_nametoindex("enp2s0");
	target.sll_halen = 6;
	target_size = sizeof(target);
	len = sizeof(buf);

	ethh = buf;
	iph = buf + sizeof(struct ethhdr);
	udph = buf + sizeof(struct iphdr) + sizeof(struct ethhdr);

	ethh->h_dest[0] = 0xd8;
	ethh->h_dest[1] = 0xcb;
	ethh->h_dest[2] = 0x8a;
	ethh->h_dest[3] = 0xf4;
	ethh->h_dest[4] = 0x72;
	ethh->h_dest[5] = 0xb1;
	/*ethh.h_source[0] = 0xb8;
	ethh.h_source[1] = 0x03;
	ethh.h_source[2] = 0x05;
	ethh.h_source[3] = 0xab;
	ethh.h_source[4] = 0x70;
	ethh.h_source[5] = 0xc5;*/
	ethh->h_source[0] = 0xe8;
	ethh->h_source[1] = 0x03;
	ethh->h_source[2] = 0x9a;
	ethh->h_source[3] = 0xb6;
	ethh->h_source[4] = 0xf2;
	ethh->h_source[5] = 0xde;
	ethh->h_proto = htons(ETH_P_IP);


	udph->check = 0;
	udph->dest = htons(atoi(argv[2]));
	udph->source = htons(60000);
	udph->len = htons(len - sizeof(struct iphdr) - sizeof(struct ethhdr));

	iph->version = 4;
	iph->ihl = sizeof(struct iphdr) / 4;
	iph->tos = 0;
	iph->tot_len = htons(len - sizeof(struct ethhdr));
	iph->id = htons(8008);
	iph->frag_off = 0x40;
	iph->ttl = 255;
	iph->protocol = IPPROTO_UDP;
	iph->saddr = inet_addr(argv[3]);
	iph->daddr = inet_addr(argv[1]);
	iph->check = checksum((__u16 *)iph, sizeof(struct iphdr));

	sprintf(buf + sizeof(struct udphdr) + sizeof(struct iphdr) + sizeof(struct ethhdr), "Hello!");
	printf("%s\n", buf + sizeof(struct udphdr) + sizeof(struct iphdr) + sizeof(struct ethhdr));

	ret = sendto(socket_fd, buf, len, 0, (struct sockaddr*)&target, target_size);
	if (ret == -1) {
		perror("send");
		return -3;
	}

	do {
		ret = recvfrom(socket_fd, buf, len, 0, (struct sockaddr *) &target, &target_size);
		printf("%d: %s\n", ntohs(udph->dest), buf + sizeof(struct udphdr) + sizeof(struct iphdr) + sizeof(struct ethhdr));
	} while (ntohs(udph->dest) != 60000);
	printf("IN: %s\n", buf + sizeof(struct udphdr) + sizeof(struct iphdr) + sizeof(struct ethhdr));
	close(socket_fd);
	return 0;
}


