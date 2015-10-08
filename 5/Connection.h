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


#define N 80
#define N_MESSAGE 2048  //used both for message and temp_buf for receive
#define HEADER_LIMIT 8193 //1 B larger than 8K, the apache header limit


class Connection
{
public:
	Connection(char* host, char* port);
	void generate_message(char* path);
	int read();
	virtual void write()=0;
	~Connection() { delete[] this->host; delete[] this->port; delete[] this->message; }

protected:
	int contain_header(char* recvbuf);
	virtual int myrecv(char* buf, int size)=0;
	char* host;
	char* port;
	char* message;
};


class Http_Conn: public Connection
{
public:
	Http_Conn(char* host, char* port): Connection(host, port){};
	void connect_to_server();
	void write();
	
private:
	int myrecv(char* buf, int size);
	int sockfd;
}; 

class Https_Conn: public Connection
{
public:
	Https_Conn(char* host, char* port): Connection(host, port){};
	void connect_to_server();
	void write();
	
	~Https_Conn() {BIO_free_all(this->conn);}
private:
	int myrecv(char* buf, int size);
	const char *openssl_strerror();
	SSL_CTX* create_ssl_context();
	BIO *open_ssl_connection(SSL_CTX *ctx, const char *server);
	int check_certificate(BIO *conn,const char *hostname);
	BIO* conn;
};