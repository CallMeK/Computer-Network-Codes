//HW4

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>

//c++ PART
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#

#define BUF_SIZE 64
#define BACKLOG 8

void read_config(std::istream& input, int& n_listeners,std::vector<int>& port_list, std::vector<std::string>& cmd_list)
{
    std::string temp_line;
    int port_temp;
    std::string cmd_temp;
    
    std::stringstream ss;
    while  (std::getline(input,temp_line))
    {
        ss << temp_line;
        ss >> port_temp;
        port_list.push_back(port_temp);
        std::getline(ss,cmd_temp);
        cmd_list.push_back(cmd_temp);
        
        //for debug
        //std::cout << port_temp << ", " << cmd_temp << std::endl;
        
        ss.clear();
        ss.str(""); // to clear the buffer
        n_listeners++; //record the number of lines directly
    }
}


void handle_client(
	int peer_sockfd, 
	const struct sockaddr_in6 *peer_addr,
	socklen_t peer_addrlen,
    const std::string& cmd
) {
	int ret;
	char host[80];
	char svc[80];

	/*
	 * use getnameinfo() to print the name of our peer
	 */
	ret = getnameinfo(
		(struct sockaddr *)peer_addr, peer_addrlen,
		host, sizeof(host),
		svc, sizeof(svc),
		NI_NUMERICSERV
	);

	if (ret != 0) {
		fprintf(
			stderr, "getnameinfo() failed: %s\n",
			gai_strerror(ret)
		);
	} else {
		fprintf(
			stderr, "[process %d] accepted connection from %s:%s\n",
			getpid(), host, svc
		);
	}
			

	/*
	 * Let's wait ten seconds before we write anything.
	 */
	sleep(2);

    dup2(peer_sockfd,STDIN_FILENO);
    dup2(peer_sockfd,STDOUT_FILENO);
    
    
	if (execl("/bin/sh","/bin/sh","-c",cmd.c_str(),NULL)< 0) {
		perror("Execution");
	}

	/* close down the socket */
	close(peer_sockfd);

}

//no change for this one
void setup_sa_nocldwait( ) {
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));

	/* continue to use the default SIGCHLD handler */
	sa.sa_handler = SIG_DFL;
	/* don't turn children into zombies */
	sa.sa_flags = SA_NOCLDWAIT;

	if (sigaction(SIGCHLD, &sa, NULL) != 0) {
		perror("sigaction");
		fprintf(stderr, "warning: failed to set SA_NOCLDWAIT\n");
	}
}

int setup_listeners(int sockets[], int n_sockets, std::vector<int>& port_list) {
	struct sockaddr_in6 bindaddr;

	/* set common fields in bindaddr */
	memset(&bindaddr, 0, sizeof(bindaddr));
	bindaddr.sin6_family = AF_INET6;
	memcpy(&bindaddr.sin6_addr, &in6addr_any, sizeof(in6addr_any));

	for (int i = 0; i < n_sockets; i++) {
		/* create socket */
		sockets[i] = socket(AF_INET6, SOCK_STREAM, 0);
		if (sockets[i] == -1) {
			perror("socket");
			return -1;
		}

		/* bind socket to port */
		bindaddr.sin6_port = htons(port_list[i]);
		if (bind(
			sockets[i],
			(struct sockaddr *) &bindaddr,
			sizeof(bindaddr)
		) != 0) {
			perror("bind");
			return -1;
		}

		/* start listening */
		if (listen(sockets[i], BACKLOG) != 0) {
			perror("listen");
			return -1;
		}
	}

	return 0;
}

void do_accept(int listen_sockfd,std::string& cmds) {
	int peer_sockfd;
	struct sockaddr_in6 src;
	socklen_t srclen;
	pid_t child;

	/* accept the connection */
	srclen = sizeof(src);
	peer_sockfd = accept(
		listen_sockfd, 
		(struct sockaddr *)&src, 
		&srclen
	);

	if (peer_sockfd < 0) {
		perror("accept");
		return;
	}

	child = fork();
	if (child == -1) {
		perror("fork");
	} else if (child == 0) {
        //child return 0 means it is in the child process
		handle_client(peer_sockfd, &src, srclen,cmds);
		exit(0);
	}

	/* 
	 * if we get here, either the fork failed or 
	 * we are in the parent. Either way, we need to
	 * close the socket.
	 */
	close(peer_sockfd);
}

int main(int argc, char *argv[]) {
    //Read the configuration file, which means the number of listeners should be dynamic
    int n_listeners;
    std::vector<int> port_list;
    std::vector<std::string> cmd_list;
    read_config(std::cin,n_listeners,port_list,cmd_list);
    

    int* listen_sockfds = new int[n_listeners];
    pollfd* pollfds = new pollfd[n_listeners];


	/* Set up SIGCHLD handler with SA_NOCLDWAIT (option 3) */
	setup_sa_nocldwait( );

	/* Set up our listening sockets. */
	setup_listeners(listen_sockfds, n_listeners, port_list);

	/* Set up our pollfds. */
    
	memset(pollfds, 0, n_listeners*sizeof(struct pollfd));
	for (int i = 0; i < n_listeners; i++) {
		pollfds[i].fd = listen_sockfds[i];
		pollfds[i].events = POLLIN;
	}

	/* Loop infinitely, accepting any connections we get. */
	for (;;) {
		/* Call select() and handle errors. */
		if (poll(pollfds, n_listeners, -1) == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				perror("poll");
				return 1;
			}
		}

		/* Iterate through fds, finding any that are ready. */
		for (int i = 0; i < n_listeners; i++) {
			if (pollfds[i].revents & POLLIN) {
				/* accept and fork the child process */
				do_accept(listen_sockfds[i],cmd_list[i]);
			}
		}
	}
    delete[] listen_sockfds;
    delete[] pollfds;
    
    

}
