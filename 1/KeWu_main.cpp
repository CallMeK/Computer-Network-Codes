#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 64

void send_message_upd_ipv6(const char* hostname, const char* port, const char* message, int sockfd)
{
	int ret;
	struct addrinfo ai_hints;
	struct addrinfo *ai_results;

	memset(&ai_hints, 0, sizeof(ai_hints));
	ai_hints.ai_family = AF_INET6;
	ai_hints.ai_socktype = SOCK_DGRAM;
	ai_hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG; /* defaults */

	ret = getaddrinfo(hostname, port, &ai_hints, &ai_results);

	if (ret != 0) {
		fprintf(stderr, "send getaddrinfo(): %s\n", gai_strerror(ret));
		return;
	}

	ret = sendto(
		sockfd, message, strlen(message), 0, 
		ai_results->ai_addr, ai_results->ai_addrlen
	);

	// printf("%d, %s\n",ret,message);

	if (ret == -1) {
		perror("sendto");
		return;
	}
}



void recv_message_udp_ipv6(char* buf,int sockfd)
{
	char host[80], svc[80];
	ssize_t size;
	struct sockaddr_in6 src;
	socklen_t srclen;

	int ret;
	
	srclen = sizeof(src);

	size = recvfrom(
		sockfd, buf, BUF_SIZE, 0,
		(struct sockaddr *)&src, &srclen
	);
	

	ret = getnameinfo(
		(struct sockaddr *)&src, sizeof(src), 
		host, sizeof(host), 
		svc, sizeof(svc), 
		0 | NI_NUMERICHOST
	);

	if (ret != 0) {
		fprintf(
			stderr, "recv getnameinfo() failed: %s\n", 
			gai_strerror(ret)
		);

	} 
}

void recv_message_udp_ipv6_infprint(char* buf_message,int sockfd)
{
	char host[80], svc[80];
	ssize_t size;
	struct sockaddr_in6 src;
	socklen_t srclen;

	int ret;
	
	srclen = sizeof(src);

	while(1)
	{
		//printf("waiting...\n");
		size = recvfrom(
			sockfd, buf_message, BUF_SIZE, 0,
			(struct sockaddr *)&src, &srclen
		);
	

		ret = getnameinfo(
			(struct sockaddr *)&src, sizeof(src), 
			host, sizeof(host), 
			svc, sizeof(svc), 
			0 | NI_NUMERICHOST
		);

		if (ret != 0) {
			fprintf(
				stderr, "recv getnameinfo() failed: %s\n", 
				gai_strerror(ret)
			);
		}
		else {
			printf("received from %s (%s):", host, svc);

			fwrite(buf_message, size, 1, stdout);
			printf("\n");
			memset(&buf_message[0], 0, BUF_SIZE);
		} 
	}
}

void parse_buf_addr_port(char* buf_address_port, char* address, char* port)
{
	int i=0;
	for(;buf_address_port[i]!=' ';i++)
	{
		address[i] = buf_address_port[i];
	}
	address[i] = '\0';
	i++; //to the char after the space
	int j =0;
	for(;buf_address_port[i]!='\0';i++,j++)
	{
		port[j] = buf_address_port[i];
	}
	port[j] = '\0';
}


int main(int argc, char* argv[])
{
	int sockfd;

	//use only one socket
	sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket");
	 	return -1;
	}



	//register
	char register_mes[80];
	sprintf(register_mes,"REGISTER %s",argv[2]);
	send_message_upd_ipv6(argv[1],"12345",register_mes,sockfd);
	
	//get others' address
	char getaddr_buf[80];
	sprintf(getaddr_buf,"GET_ADDR %s",argv[3]);
	send_message_upd_ipv6(argv[1],"12345",getaddr_buf,sockfd);
	
	char buf_address_port[BUF_SIZE]={0};
	char address[BUF_SIZE]={0};
	char port[BUF_SIZE]={0};
	recv_message_udp_ipv6(buf_address_port,sockfd);
	
	// printf("Got the address\n");
	if(strcmp(buf_address_port,"NOT FOUND")!=0)
	{
		
		parse_buf_addr_port(buf_address_port,address,port);

		//send others message
		send_message_upd_ipv6(address,port,argv[4],sockfd);
	}
	else
	{
		printf("Not Found\n");
	}
	//printf("%s\n",address);
	//printf("%s\n",port);


	// printf("Message Sent\n");


	// recieve message
	char buf_message[BUF_SIZE]={0};
	recv_message_udp_ipv6_infprint(buf_message,sockfd);
	close(sockfd);
}


