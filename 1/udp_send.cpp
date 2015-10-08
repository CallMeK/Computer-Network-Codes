/*
 * udp_send.cpp
 * 
 * Send a UDP packet.
 */

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char *argv[]) {
	int sockfd, ret;
	struct sockaddr_in dest;

	dest.sin_family = AF_INET;
	dest.sin_port = htons(12345);
	if (inet_aton(argv[1], &dest.sin_addr) == 0) {
		fprintf(stderr, "invalid address!\n");
		return 1;
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket");
		return 1;
	}

	ret = sendto(
		sockfd, "Hello World", 11, 0, 
		(struct sockaddr *)&dest, sizeof(dest)
	);
	if (ret == -1) {
		perror("sendto");
		return 1;
	}
}
