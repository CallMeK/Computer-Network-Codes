#include "Connection.h"

Connection::Connection(char* host, char* port)
{
	this->host = new char[N];
	this->port = new char[N];
	this->message = new char[N_MESSAGE];
	std::fill(this->host,this->host+N,0);
	std::fill(this->port,this->port+N,0);
	std::fill(this->message,this->message+N_MESSAGE,0);
	memcpy(this->host,host,N);
	memcpy(this->port,port,N);
}

void Connection::generate_message(char* path)
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

	sprintf(this->message,"GET %s HTTP/1.1\r\nHost: %s:%s\r\nUser-Agent: wuk3-netprog-hw3/1.0\r\nConnection: close\r\n\r\n",
		path_temp,this->host,this->port);

	// printf("%s",this->message);
}

int Connection::contain_header(char* recvbuf)
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


int Connection::read()
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
			ret = this->myrecv(temp_buf, N_MESSAGE);
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
					body_pos = this->contain_header(header);
					//printf("body position: %d\n", body_pos);
					if(body_pos != -1)
					{
						headerfound = true;
						fwrite(header,sizeof(char),body_pos-4,stderr); //The print doesn't include the /r/n/r/n
						if(body_pos < header_size)
						{
							// we have some body data in the header buffer, need to print it and make sure the size is substracted
							fwrite(header+body_pos,sizeof(char),header_size - body_pos,stdout);
						}
					}
				}
				else
				{	
					//now we have got the header,
					fwrite(temp_buf,sizeof(char),ret,stdout);
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


void Http_Conn::connect_to_server()
{
	// From the exmple code from the class
	int ret, success;
	struct addrinfo ai_hints;
	struct addrinfo *ai_results, *j;
	
	/* create an IPv6 TCP socket */
	this->sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket");
		return;
	}

	/* find TCP IPv6 addresses from command line arg */
	memset(&ai_hints, 0, sizeof(ai_hints));
	ai_hints.ai_family = AF_INET6;
	ai_hints.ai_socktype = SOCK_STREAM;
	ai_hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG; /* defaults */

	ret = getaddrinfo(this->host, this->port, &ai_hints, &ai_results);

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
		ret = connect(this->sockfd, j->ai_addr, j->ai_addrlen);
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

void Http_Conn::write()
{
	ssize_t ret, sent = 0;
	uint8_t *bytes = (uint8_t *)this->message;
	ssize_t size = strlen(this->message);

	while (size > 0) {
		ret = send(this->sockfd, bytes, size, 0);
		//printf("size %u; sent: %zd\n",size,ret);
		if (ret < 0) {
			std::cout << "Failed to send\n";
			return;
		} else {
			size -= ret;
			bytes += ret;
			sent += ret;
		}
	}

}

int Http_Conn::myrecv(char* buf, int size)
{
	return recv(this->sockfd,buf,size,0);
}

//HTTPS
void Https_Conn::connect_to_server()
{
	SSL_CTX *ctx;

	SSL_library_init( );
	SSL_load_error_strings( );

	/* Create the OpenSSL context */
	ctx = this->create_ssl_context( );
	if (ctx == NULL) {
		fprintf(stderr, "Failed to create SSL context\n");
		return;
	}

	std::string hostname(this->host);
	std::string port(this->port);
	std::string destination = hostname + ":" + port;
	/* Try to open an SSL connection */
	this->conn = this->open_ssl_connection(ctx, destination.c_str());
	if (conn == NULL) {
		fprintf(stderr, "Failed to create SSL connection\n");
		SSL_CTX_free(ctx);
		return;
	}

	if (this->check_certificate(this->conn, hostname.c_str( )) != 0) {
		fprintf(stderr, "Certificate tests failed\n");
		BIO_free_all(conn);
		SSL_CTX_free(ctx);
		return;
	}
}


const char * Https_Conn::openssl_strerror( ) {
	return ERR_error_string(ERR_get_error(), NULL);
}

SSL_CTX * Https_Conn::create_ssl_context( ) {
	SSL_CTX *ret;

	/* create a new SSL context */
	ret = SSL_CTX_new(SSLv23_client_method( ));
	
	if (ret == NULL) {
		fprintf(stderr, "SSL_CTX_new failed!\n");
		return NULL;
	}

	/* 
	 * set our desired options 
	 *
	 * We don't want to talk to old SSLv2 or SSLv3 servers because
	 * these protocols have security issues that could lead to the
	 * connection being compromised. 
	 *
	 * Return value is the new set of options after adding these 
	 * (we don't care).
	 */
	SSL_CTX_set_options(
		ret, 
		SSL_OP_NO_SSLv2 | 
		SSL_OP_NO_SSLv3 |
		SSL_OP_NO_COMPRESSION
	);

	/*
	 * set up certificate verification
	 *
	 * We want the verification to fail if the peer doesn't 
	 * offer any certificate. Otherwise it's easy to impersonate
	 * a legitimate server just by offering no certificate.
	 *
	 * No error checking, not because I'm being sloppy, but because
	 * these functions don't return error information.
	 */
	SSL_CTX_set_verify(
		ret, 
		SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
		NULL
	);
	SSL_CTX_set_verify_depth(ret, 4);

	/*
	 * Point our context at the root certificates.
	 * This may vary depending on your system.
	 */
	if (SSL_CTX_load_verify_locations(ret, NULL, "/usr/local/etc/openssl/certs") == 0) {
		fprintf(stderr, "Failed to load root certificates\n");
		SSL_CTX_free(ret);	
		return NULL;
	}

	return ret;
}

BIO * Https_Conn::open_ssl_connection(SSL_CTX *ctx, const char *server) {
	BIO *ret;

	/* use our settings to create a BIO */
	ret = BIO_new_ssl_connect(ctx);
	if (ret == NULL) {
		fprintf(	
			stderr, 
			"BIO_new_ssl_connect failed: %s\n",
			openssl_strerror( )
		);
		return NULL;
	}

	/* according to documentation, this cannot fail */
	BIO_set_conn_hostname(ret, server);

	/* try to connect */
	if (BIO_do_connect(ret) != 1) {
		fprintf(stderr, 
			"BIO_do_connect failed: %s\n",
			openssl_strerror( )
		);

		BIO_free_all(ret);	
		return NULL;
	}

	/* try to do TLS handshake */
	if (BIO_do_handshake(ret) != 1) {
		fprintf(
			stderr, 
			"BIO_do_handshake failed: %s\n",
			openssl_strerror( )
		);

		BIO_free_all(ret);
		return NULL;
	}

	return ret;
}

int Https_Conn::check_certificate(BIO *conn, const char *hostname) {
	SSL *ssl;
	X509 *cert;
	X509_NAME *subject_name;
	X509_NAME_ENTRY *cn;
	ASN1_STRING *asn1;
	unsigned char *cn_str;
	int pos;
	bool hostname_match;

	/* get this particular connection's TLS/SSL data */
	BIO_get_ssl(conn, &ssl);
	if (ssl == NULL) {
		fprintf(
			stderr, "BIO_get_ssl failed: %s\n",
			openssl_strerror( )
		);

		return -1;
	}

	/* get the connection's certificate */
	cert = SSL_get_peer_certificate(ssl);
	if (cert == NULL) {
		/* no certificate was given - failure */
		return -1;
	}

	/* check that the certificate was verified */
	if (SSL_get_verify_result(ssl) != X509_V_OK) {
		/* certificate was not successfully verified */
		return -1;
	}

	/* get the name of the certificate subject */
	subject_name = X509_get_subject_name(cert);
	
	/* and print it out */
	X509_NAME_print_ex_fp(stderr, subject_name, 0, 0);

	/* loop through "common names" (hostnames) in cert */
	pos = -1;
	hostname_match = false;
	for (;;) {
		/* move to next CN entry */
		pos = X509_NAME_get_index_by_NID(
			subject_name, NID_commonName, pos
		);

		if (pos == -1) { 
			break;
		}

		cn = X509_NAME_get_entry(subject_name, pos);
		asn1 = X509_NAME_ENTRY_get_data(cn);
		if (ASN1_STRING_to_UTF8(&cn_str, asn1) < 0) {
			fprintf(
				stderr, "ASN1_STRING_to_UTF8 failed: %s",
				openssl_strerror( )
			);
			return -1;
		}

		/* finally we have a hostname string! */
		if (strcmp((char *) cn_str, hostname) == 0) {
			hostname_match = true;
		}
	}

	if (hostname_match) {
		return 0;
	} else {
		fprintf(stderr, "hostnames do not match!\n");
		return -1;
	}
}


void Https_Conn::write()
{
	BIO_puts(this->conn, this->message);
}

int Https_Conn::myrecv(char* buf, int size)
{
	return BIO_read(this->conn, buf, size);
}













