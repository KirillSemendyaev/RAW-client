#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/udp.h>
#include <linux/types.h>

int main(int argc, char **argv)
{
	if (argc != 3) {
		perror("args");
		return -2;
	}
	int socket_fd, ret;
	size_t len;
	struct sockaddr_in target;
	struct udphdr udph;
	socklen_t target_size;

	char buf[64] = {0};

	socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (socket_fd == -1) {
		perror("socket");
		return -1;
	}
	
	memset(&target, 0, sizeof(target));
	target.sin_family = AF_INET;
	target.sin_port = htons(atoi(argv[2]));
	target.sin_addr.s_addr = inet_addr(argv[1]);
	target_size = sizeof(target);
	sprintf(buf + 8, "Hello!");
	len = sizeof(buf);
	udph.check = 0;
	udph.dest = htons(atoi(argv[2]));
	udph.source = htons(60000);
	udph.len = htons(len);
	memcpy(buf, &udph, sizeof(udph));
	printf("%s\n", buf);

	ret = sendto(socket_fd, buf, len, 0, (struct sockaddr*)&target, target_size);
	if (ret == -1) {
		perror("send");
		return -3;
	}
	__u16 dport;
	do {
		ret = recvfrom(socket_fd, buf, len, 0, (struct sockaddr *) &target, &target_size);
//		ret = recvfrom(socket_fd, buf, len, 0, NULL, NULL);
		memcpy(&dport, buf + 22, 2);
		dport = ntohs(dport);
		printf("%d: %s\n", dport, buf + 28);
	} while (dport != 60000);
	printf("IN: %s\n", buf + 28);
	close(socket_fd);
	return 0;
}


