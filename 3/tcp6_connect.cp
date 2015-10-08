/*
 * tcp6_connect.cpp
 * 
 * 
 */

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <netdb.h>
#include <string.h>

/*
 * send all data, even if it doesn't all go at once
 */
int send_all(int fd, const void *data, size_t size, int flags) {
	ssize_t ret, sent = 0;
	uint8_t *bytes = (uint8_t *)data;

	while (size > 0) {
		ret = send(fd, bytes, size, flags);
		printf("size %u; sent: %zd\n",size,ret);
		if (ret < 0) {
			return ret;
		} else {
			size -= ret;
			bytes += ret;
			sent += ret;
		}
	}

	return sent;
}

int main(int argc, char *argv[]) {
	int sockfd, ret, success;
	struct addrinfo ai_hints;
	struct addrinfo *ai_results, *j;

	char recvbuf[80];
	
	/* create an IPv6 TCP socket */
	sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket");
		return 1;
	}

	/* find TCP IPv6 addresses from command line arg */
	memset(&ai_hints, 0, sizeof(ai_hints));
	ai_hints.ai_family = AF_INET6;
	ai_hints.ai_socktype = SOCK_STREAM;
	ai_hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG; /* defaults */

	ret = getaddrinfo(argv[1], "80", &ai_hints, &ai_results);

	if (ret != 0) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(ret));
		return 1;
	}

	/*
	 * Loop through the addresses from getaddrinfo(). Try to connect()
	 * to each until we find one that works.
	 */
	success = 0;
	for (j = ai_results; j != NULL; j = j->ai_next) {
		ret = connect(sockfd, j->ai_addr, j->ai_addrlen);
		if (ret == 0) {
			success = 1;
			break;
		}
	}

	freeaddrinfo(ai_results);

	if (success == 0) {
		perror("connect");
		return 1;
	}

	/* 
	 * at this point, we have a connected TCP socket.
	 * Let's send a message.
	 */
	ret = send_all(sockfd, "Hello World\n", 12, 0);
	if (ret < 0) {
		perror("send_all");
		return 1;
	}

	/*
	 * Let's see if we get anything back.
	 */
	ret = recv(sockfd, recvbuf, sizeof(recvbuf), 0);
	if (ret < 0) {
		perror("recv");
		return 1;
	}

	fwrite(recvbuf, ret, 1, stdout);
	
	return 0;
}
