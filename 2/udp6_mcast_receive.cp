/*
 * udp6_mcast_receive.cpp
 * 
 * Subscribe to and receive UDPv6 multicast packets.
 */

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 64

int main(int argc, char *argv[]) {
	int sockfd;
	struct sockaddr_in6 src, bindaddr;
	struct ipv6_mreq mreq;

	socklen_t srclen;
	char buf[BUF_SIZE];
	ssize_t size;
	int ret;
	char host[80], svc[80];

	//const char *multicast_addr = "ff1e::0123:4567:89ab:cdef";
	const char *multicast_addr = "::ffff:239.11.22.33";


	sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket");
		return 1;
	}

	/* bind to port 12345 on all interfaces */
	memset(&bindaddr, 0, sizeof(bindaddr));
	bindaddr.sin6_family = AF_INET6;
	bindaddr.sin6_port = htons(12345);
	memcpy(&bindaddr.sin6_addr, &in6addr_any, sizeof(in6addr_any));

	if (bind(sockfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) != 0) {
		perror("bind");
		return 1;
	}

	/* subscribe to multicasts to the address of interest */
	if (inet_pton(AF_INET6, multicast_addr, &mreq.ipv6mr_multiaddr) != 1) {
		fprintf(stderr, "inet_pton failed\n");
		return 1;
	}

	mreq.ipv6mr_interface = 0;

	ret = setsockopt(
		sockfd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
		&mreq, sizeof(mreq)
	);

	if (ret != 0) {
		perror("setsockopt");
		return 1;
	}

	for (;;) {
		srclen = sizeof(src);
		size = recvfrom(
			sockfd, buf, BUF_SIZE, 0,
			(struct sockaddr *)&src, &srclen
		);

		if (size == -1) {
			perror("recvfrom");
			continue;
		}

		ret = getnameinfo(
			(struct sockaddr *)&src, sizeof(src), 
			host, sizeof(host), 
			svc, sizeof(svc), 
			NI_NUMERICHOST
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
	}
}
