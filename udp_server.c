#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	if (argc != 3) {
		printf("Usage: server [address] [port]\n");
		return -1;
	}

	int socket_fd, ret;
	size_t len;
	char buf[64] = {0};
	len = sizeof(buf);
	socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (socket_fd == -1) {
		perror("socket");
		return -2;
	}
	
	struct sockaddr_in target, server;
	socklen_t target_size = sizeof(target), server_size = sizeof(server);
	memset(&server, 0, server_size);
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);
	ret = bind(socket_fd, (struct sockaddr*)&server, server_size);

	if (ret == -1) {
		perror("bind");
		return -3;
	}
	while (1)	{
		memset(&target, 0, target_size);
		recvfrom(socket_fd, buf, len, 0, (struct sockaddr*)&target, &target_size);
		printf("Connection from %s:%d\n", inet_ntoa(target.sin_addr), ntohs(target.sin_port));
		printf("IN: %s\n", buf);
		strcpy(buf + 5, " world!");
		sleep(20);
		sendto(socket_fd, buf, len, 0, (struct sockaddr*)&target, target_size);
		printf("OUT: %s\n", buf);
		printf("Replied\n");
	}
	close(socket_fd);

	return 0;
}



