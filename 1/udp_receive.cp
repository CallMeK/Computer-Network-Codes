/*
 * udp_receive.cpp
 * 
 * Receive a UDP packet and print its payload.
 */

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 64

int main(int argc, char *argv[]) {
	int sockfd;
	struct sockaddr_in src, bindaddr;
	socklen_t srclen;
	char buf[BUF_SIZE];
	ssize_t size;

	bindaddr.sin_family = AF_INET;
	bindaddr.sin_port = htons(12345);
	bindaddr.sin_addr.s_addr = INADDR_ANY;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket");
		return 1;
	}

	if (bind(sockfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) != 0) {
		perror("bind");
		return 1;
	}

	srclen = sizeof(src);
	size = recvfrom(
		sockfd, buf, BUF_SIZE, 0,
		(struct sockaddr *)&src, &srclen
	);

	if (size == -1) {
		perror("sendto");
		return 1;
	}

	fwrite(buf, size, 1, stdout);
	close(sockfd);
}
