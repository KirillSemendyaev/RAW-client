#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <linux/types.h>

int main(int argc, char **argv)
{
	if (argc != 4) {
		printf("Usage: ./RAW_CLIENT [dest_ip] [port] [source_ip]\n");
		return -2;
	}

	char buf[512] = {0};
	int socket_fd, ret, on = 1;
	size_t len;
	struct sockaddr_in target;
	struct udphdr udph, *udp_ptr;
	struct iphdr iph, *ip_ptr;
	socklen_t target_size;

	socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (socket_fd == -1) {
		perror("socket");
		return -1;
	}

	if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
		perror("setsockopt-hdrincl");
		return -4;
	}
	
	memset(&target, 0, sizeof(target));
	memset(&iph, 0, sizeof(iph));
	memset(&udph, 0, sizeof(udph));
	target.sin_family = AF_INET;
	target.sin_port = htons(atoi(argv[2]));
	target.sin_addr.s_addr = inet_addr(argv[1]);
	target_size = sizeof(target);
	len = sizeof(buf);

	udph.check = 0;
	udph.dest = htons(atoi(argv[2]));
	udph.source = htons(60000);
	udph.len = htons(len - sizeof(iph));

	iph.version = 4;
	iph.ihl = sizeof(iph) / 4;
	iph.tos = 0;
	iph.tot_len = htons(len);
	iph.id = htons(8008);
	iph.frag_off = 0;
	iph.ttl = 255;
	iph.protocol = IPPROTO_UDP;
	iph.saddr = inet_addr(argv[3]);
	iph.daddr = inet_addr(argv[1]);
	iph.check = 0;

	memcpy(buf, &iph, sizeof(iph));
	memcpy(buf + sizeof(iph), &udph, sizeof(udph));
	sprintf(buf + sizeof(udph) + sizeof(iph), "Hello!");
	printf("%s\n", buf + sizeof(udph) + sizeof(iph));

	ret = sendto(socket_fd, buf, len, 0, (struct sockaddr*)&target, target_size);
	if (ret == -1) {
		perror("send");
		return -3;
	}

	do {
		ret = recvfrom(socket_fd, buf, len, 0, (struct sockaddr *) &target, &target_size);
		udp_ptr = buf + sizeof(iph);
		ip_ptr = buf;
		printf("%d: %s\n", ntohs(udp_ptr->dest), buf + sizeof(udph) + sizeof(iph));
	} while (ntohs(udp_ptr->dest) != 60000);
	printf("IN: %s\n", buf + sizeof(udph) + sizeof(iph));
	close(socket_fd);
	return 0;
}


