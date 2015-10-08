/*
 * udp_receive_with_name.cpp
 * 
 * Receive a UDP packet, and use getnameinfo() to print the address.
 */

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define BUF_SIZE 64

int main(int argc, char *argv[]) {
	int sockfd;
	struct sockaddr_in src, bindaddr;
	socklen_t srclen;
	char buf[BUF_SIZE];
	ssize_t size;
	int ret;
	char host[80], svc[80];

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
		perror("recvfrom");
		return 1;
	}

	ret = getnameinfo(
		(struct sockaddr *)&src, sizeof(src), 
		host, sizeof(host), 
		svc, sizeof(svc), 
		0
	);

	if (ret != 0) {
		fprintf(
			stderr, "getnameinfo() failed: %s\n", 
			gai_strerror(ret)
		);
	} else {
		printf("received from %s (%s):", host, svc);
	}

	fwrite(buf, size, 1, stdout);
	printf("\n");
	close(sockfd);
}
