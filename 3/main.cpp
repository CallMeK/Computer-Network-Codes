#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <netdb.h>
#include <string.h>
#include <string>

#include <iostream> // only for debug

#define N 80
#define N_MESSAGE 2048  //used both for message and temp_buf for receive
#define HEADER_LIMIT 8193 //1 B larger than 8K, the apache header limit



void parse_URL(char* url, char* host, char* port, char* path)
{
	//I will use the simplest way to do it
	int i=0,i_host=0,i_port=0,i_path=0;
	bool for_host, for_port, for_path;
	for_host=false;
	for_port=false;
	for_path=false;
	int colon_counter = 0;
	int back_slash_counter = 0;
	for( i = 0; i < strlen(url); i++ )
	{


		
		if(url[i] == ':') colon_counter++;
		if(url[i] == '/') back_slash_counter++;


		if(back_slash_counter==2  && colon_counter==1)
		{
			for_host=true;
		}
		
		if(back_slash_counter == 3)
		{
			for_host = false;
			for_port = false;
			for_path = true;
		}

		if(back_slash_counter == 2 && colon_counter==2)
		{
			for_host = false;
			for_port = true;
		}



		if(for_host && url[i]!='/')
		{
			host[i_host]=url[i];
			i_host++;
		}

		if(for_port && url[i]!=':')
		{
			port[i_port]=url[i];
			i_port++;
		}
		
		if(for_path)
		{
			path[i_path] = url[i];
			i_path++;
		}

	}
	//add the terminator
	host[i_host]='\0';
	port[i_port]='\0';
	path[i_path]='\0';
	if(strlen(port) == 0)
	{
		sprintf(port,"80");
	}

}


void connect_to_server(int& sockfd, char* host, char* port)
{
	// From the exmple code from the class
	int ret, success;
	struct addrinfo ai_hints;
	struct addrinfo *ai_results, *j;
	
	/* create an IPv6 TCP socket */
	sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket");
		return;
	}

	/* find TCP IPv6 addresses from command line arg */
	memset(&ai_hints, 0, sizeof(ai_hints));
	ai_hints.ai_family = AF_INET6;
	ai_hints.ai_socktype = SOCK_STREAM;
	ai_hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG; /* defaults */

	ret = getaddrinfo(host, port, &ai_hints, &ai_results);

	if (ret != 0) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(ret));
		return;
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

	//std::cout << "successful connected\n"; 
	freeaddrinfo(ai_results);

	if (success == 0) {
		perror("connect");
		return;
	}
}

void generate_message(char* message, char* host, char* port, char* path)
{
	const char* path_temp; // since the deprecation of char* from string in c++11
	if(strlen(path)==0)
	{
		path_temp="/";
	}
	else
	{
		path_temp=path;
	}

	sprintf(message,"GET %s HTTP/1.1\r\nHost: %s:%s\r\nUser-Agent: wuk3-netprog-hw3/1.0\r\n\r\n",path_temp,host,port);

}



int send_all(int fd, const void *data, size_t size, int flags) {
	ssize_t ret, sent = 0;
	uint8_t *bytes = (uint8_t *)data;

	while (size > 0) {
		ret = send(fd, bytes, size, flags);
		//printf("size %u; sent: %zd\n",size,ret);
		if (ret < 0) {
			std::cout << "Failed to send\n";
			return ret;
		} else {
			size -= ret;
			bytes += ret;
			sent += ret;
		}
	}

	//std::cout << "All sent\n";
	return sent;
}

int contain_header(char* recvbuf)
{
	//check if '\r\n\r\n' exists
	for(unsigned int i=0;recvbuf[i+3]!='\0';i++)
	{
		if(recvbuf[i]=='\r' &&
			recvbuf[i+1] == '\n' &&
			recvbuf[i+2] == '\r' &&
			recvbuf[i+3] == '\n' )
		{
			return i+4;
		}
	}
	return -1;
}

int find_ContentLength(char* header)
{
	// return the response body length, if there is no such header, return -1
	std::string header_str(header);
	char number[10]={0};
	int j=0;
	std::size_t found = header_str.find("Content-Length: ");
	if (found == std::string::npos)
	{
		return -1;
	}
	for(unsigned int i=found; i<strlen(header); i++)
	{
		if(isdigit(header_str[i]))
		{
			number[j] = header_str[i];
			j++;
		}
		else if(j!=0)
			break;
	}
	return atoi(number);
}


int recv_print(int sockfd)
{
	//receive and print the message
	//the header should be not be larger than 8KB, as required by apache
	//So I am not trying to store the whole header
	//To avoid the broken stack, even though the stack should be much larger, I use alloc here.
		char* temp_buf = (char *)calloc(N_MESSAGE,sizeof(char));
		char* header = (char *)calloc(HEADER_LIMIT,sizeof(char));
		bool headerfound = false;
		int ret,body_pos,header_size=0;
		int recv_length;

		do{
			memset(temp_buf,0,N_MESSAGE);
			ret = recv(sockfd, temp_buf, N_MESSAGE, 0);
			if (ret > 0) 
			{

				if(!headerfound)
				{
					if (header_size+ret >= HEADER_LIMIT)
					{
						throw "The header size is larger than header limit. Abnormal situation\n";
					}
					memcpy(header+header_size,temp_buf,ret*sizeof(char));
					header_size = header_size + ret; //the size of the header
					body_pos = contain_header(header);
					//printf("body position: %d\n", body_pos);
					if(body_pos != -1)
					{
						headerfound = true;
						fwrite(header,sizeof(char),body_pos-4,stderr); //The print doesn't include the /r/n/r/n
						recv_length = find_ContentLength(header);
						if(recv_length == -1)
						{
							throw "No Content-Length header! Exit\n";
						}	
						if(body_pos < header_size)
						{
							// we have some body data in the header buffer, need to print it and make sure the size is substracted
							fwrite(header+body_pos,sizeof(char),header_size - body_pos,stdout);
							recv_length = recv_length - (header_size - body_pos);
						}
					}
				}
				else
				{	
					//now we have got the header, and also the recv_length
					if(recv_length>ret)
					{
						fwrite(temp_buf,sizeof(char),ret,stdout);
						recv_length = recv_length - ret;
					}
					else
					{
						fwrite(temp_buf,sizeof(char),recv_length,stdout);
						break;	//  We have got what we want, no need to wait
					}
				}

			}
			else if(ret == 0)
			{
				if(header_size == 0)
				{
					perror("recv");
					return 0;
				}
			}
			if (ret < 0) {
				perror("recv");
				return 0;
			}
		}while(ret>0);


	//fwrite(recvbuf, sizeof(char), strlen(recvbuf), stdout);
	//printf("%d\n",(int)strlen(recvbuf));
	free(temp_buf);
	free(header);
	return recv_length;
}


int main(int argc,char* argv[])
{
	char* url = argv[1]; //no real need to do that, but make the code clearer a littile bit..
	char host[N];
	char port[N];
	char path[N];
	parse_URL(url,host,port,path);
	//printf("host: %s; port: %s; path: %s\n",host,port,path);

	//get address and connect
	int sockfd;
	connect_to_server(sockfd,host,port); // get the sockfd
	char  message[N_MESSAGE];
	generate_message(message,host,port,path);
	send_all(sockfd,message,strlen(message),0);
	recv_print(sockfd);

}




