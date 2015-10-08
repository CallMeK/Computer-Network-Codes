#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <netdb.h>
#include <string.h>
#include <string>

#include <iostream> // only for debug


//https part
#include <openssl/ssl.h>
#include <openssl/err.h>
//end


//The classes
#include "Connection.h"
//end

#define N 80
#define N_MESSAGE 2048  //used both for message and temp_buf for receive
#define HEADER_LIMIT 8193 //1 B larger than 8K, the apache header limit



void parse_URL(char* url, char* host, char* port, char* path, char* scheme)
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
		if(!for_host && !for_port && !for_path && url[i]!= ':')
		{
			scheme[i] = url[i];
		}
		
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
	//add the terminator, just for safety
	host[i_host]='\0';
	port[i_port]='\0';
	path[i_path]='\0';
	if(strlen(port) == 0)
	{
		if(strcmp(scheme,"http")==0)
		{
			sprintf(port,"80");
		}
		else if(strcmp(scheme,"https")==0)
		{
			sprintf(port,"443");
		}
	}
}




int main(int argc,char* argv[])
{
	char* url = argv[1]; //no real need to do that, but make the code clearer a littile bit..
	char host[N]={0};
	char port[N]={0};
	char path[N]={0};

	char protocol[N]={0}; //either http or https
	parse_URL(url,host,port,path,protocol);
	// printf("%s\n%s\n%s\n%s\n",protocol,host,port,path);
	if (strcmp(protocol,"http") == 0)
	{
	//printf("host: %s; port: %s; path: %s\n",host,port,path);
		Http_Conn httpconn(host,port);
		httpconn.generate_message(path);
		httpconn.connect_to_server(); // get the sockfd
		httpconn.write();
		httpconn.read();
	}
	else if(strcmp(protocol,"https") == 0)
	{
		Https_Conn httpsconn(host,port);
		httpsconn.generate_message(path);
		httpsconn.connect_to_server();
		httpsconn.write();
		httpsconn.read();
	}

}




