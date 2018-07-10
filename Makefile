all:	
	gcc -Wall -o UDP_SERVER udp_server.c
	gcc -Wall -o RAW_CLIENT raw_client.c
clean:
	rm RAW_CLIENT UDP_SERVER

