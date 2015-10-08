/*
 * udp6_send.cpp
 * 
 * Send a UDP packet. Use getaddrinfo() to get the address.
 */

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <netdb.h>
#include <string.h>

int main(int argc, char *argv[]) {
	int sockfd, ret;
	struct addrinfo ai_hints;
	struct addrinfo *ai_results;


	/* create an IPv6 UDP socket */
	sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket");
		return 1;
	}

	/* find UDP IPv6 addresses from command line arg */
	memset(&ai_hints, 0, sizeof(ai_hints));
	ai_hints.ai_family = AF_INET6;
	ai_hints.ai_socktype = SOCK_DGRAM;
	ai_hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG; /* defaults */

	ret = getaddrinfo(argv[1], "12345", &ai_hints, &ai_results);

	if (ret != 0) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(ret));
		return 1;
	}

	ret = sendto(
		sockfd, "Hello World", 11, 0, 
		ai_results->ai_addr, ai_results->ai_addrlen
	);
	if (ret == -1) {
		perror("sendto");
		return 1;
	}
}
