//
//  main.cpp
//  Project2
//
//  Created by WuKe on 4/6/15.
//  Copyright (c) 2015 WuKe. All rights reserved.
//


#include <iostream>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

//C++
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <thread>

#define N_REQ 1024
#define N_HOST 80


void setup_sock(int& main_sockfd,const int port)
{
    //bind the sock to certain port
    struct sockaddr_in6 bindaddr;
    memset(&bindaddr, 0, sizeof(bindaddr));
    bindaddr.sin6_family = AF_INET6;
    bindaddr.sin6_port = htons(port);
    memcpy(&bindaddr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    
    main_sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (main_sockfd == -1) {
        perror("socket");
        return;
    }
    
    if (bind(main_sockfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) != 0) {
        perror("bind");
        return;
    }
}

void recv_message(char* request, int sockfd, char* host, char* svc)
{
    //set request in request and set host and svc
    struct sockaddr_in6 src;
    socklen_t srclen;
    ssize_t size;
    int ret;
    srclen = sizeof(src);
    size = recvfrom(sockfd, request, N_REQ, 0, (struct sockaddr *)&src, &srclen);
    // printf("req : %s\n",request);
    
    
    if (size == -1) {
        perror("recvfrom");
        return;
    }
   
    ret = getnameinfo(
                      (struct sockaddr *)&src, srclen,
                      host, N_HOST,
                      svc, N_HOST,
                      0 | NI_NUMERICHOST
                      );
    // printf("from: %s %s\n",host,svc);
    if (ret != 0) {
        fprintf(
                stderr, "getnameinfo() failed: %s\n",
                gai_strerror(ret)
                );
    }
    
}

void send_message(const char* host, const char* svc, int sockfd, const char* message, size_t size)
{
    struct addrinfo ai_hints;
    struct addrinfo *ai_results;
    int ret;
    
   	memset(&ai_hints, 0, sizeof(ai_hints));
    ai_hints.ai_family = AF_INET6;
    ai_hints.ai_socktype = SOCK_DGRAM;
    ai_hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG; /* defaults */
    
    ret = getaddrinfo(host,svc, &ai_hints, &ai_results);
    
    if (ret != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(ret));
        return;
    }
    
    ret = sendto(
                 sockfd, message, size, 0,
                 ai_results->ai_addr, ai_results->ai_addrlen
                 );
    if (ret == -1) {
        perror("send message failed");
        return;
    }
}




std::string req_type(char* request)
{
    //size should always make sense because the error is already handled in recv_message
    std::stringstream temp;
    temp << request;
    std::string header;
    temp >> header;
    return header;
    
}

typedef std::map<std::string, std::vector<std::string> > reg_table_type;

bool is_not_alnum(char c)
{
    //tiny function to determin the char is not alnum, coupled with find_if
    return !isalnum(c);
}



void register_user(char* request, int sockfd, reg_table_type& register_table,char* host,char* svc)
{
    //the parse is done in each funciton,since there is no general way to do the parsing.
    std::stringstream temp;
    temp << request;
    std::string header,user_id;
    temp >> header >> user_id;
    //now determine if user_id is alnum
    if(std::find_if(user_id.begin(), user_id.end(), is_not_alnum) != user_id.end())
    {
        perror("User id is not alphanumeric");
        return;
    }

    register_table[user_id].push_back(std::string(host));
    register_table[user_id].push_back(std::string(svc)); //always update the address without checking
    
    char message[N_REQ];
    sprintf(message, "ACK_REGISTER %s", user_id.c_str());
    send_message (host,svc,sockfd, message,13+user_id.length());
}


void parse_call_req(char* request,std::string& user_id, std::string& peer_id)
{
    std::string temp, temp1,temp2;
    //need to check if peer_id is registered
    std::stringstream ss;
    ss << request;
    ss >> temp >> temp1 >> temp2;
    
    //some error handling
    if (temp1.find("FROM:")!=0) {
        perror("Call request is wrong");
    }
    if (temp2.find("TO:")!=0) {
        perror("Call request is wrong");
    }
    user_id = temp1.substr(temp1.find(":")+1);
    peer_id = temp2.substr(temp2.find(":")+1);

}
    

void process_call(char* request,int sockfd,reg_table_type& register_table)
{
    std::string host_user, svc_user, host_peer, svc_peer, user_id, peer_id;
    parse_call_req(request,user_id,peer_id);
    
    if (register_table.find (user_id) == register_table.end()) {
        perror("The user is not registered");
        return;
    }
    host_user = register_table[user_id][0];
    svc_user = register_table[user_id][1];
    
    if (register_table.find(peer_id) == register_table.end()) {
        //peer id not found
        send_message(host_user.c_str() , svc_user.c_str(), sockfd, "CALL_FAILED unknown peer", 24);
    }
    else{
        host_peer = register_table[peer_id][0];
        svc_peer = register_table[peer_id][1];
        send_message(host_peer.c_str(), svc_peer.c_str(), sockfd, request, strlen(request));
    }
}


void pass_call(char* request, int sockfd, reg_table_type& register_table)
{
    std::string host_user, svc_user, host_peer, svc_peer, user_id,peer_id;
    parse_call_req(request,user_id,peer_id);
    
    host_user = register_table[user_id][0];
    svc_user = register_table[user_id][1];
    
    host_peer = register_table[peer_id][0];
    svc_peer = register_table[peer_id][1];
    
    //The message is actually from the recipient to original user
    //So user is now recipient and original user is now peer
    send_message(host_peer.c_str(), svc_peer.c_str(), sockfd, request, strlen(request));
}

void start_session(int sockfd, const std::string& host_user, const std::string& host_peer)
{
    // printf("Starting the session\n");
    // std::cout << host_user << " " << host_peer << "\n";
    char host[N_HOST]={0}, svc[N_HOST]={0};
    std::string svc_user, svc_peer;

    char message[N_REQ]={0};
    bool user_match = false;
    bool peer_match = false;
    
    while (!user_match || !peer_match) {
        recv_message(message, sockfd, host,svc);
        if (!user_match && 
            std::string(host) == host_user &&
            svc_peer!=std::string(svc) ) 
        {
            svc_user = std::string(svc);
            user_match = true;
        }
        
        if (!peer_match && 
            std::string(host) == host_peer && 
            svc_user!=std::string(svc) ) 
        {   
            svc_peer = std::string(svc);
            peer_match = true;
        }

        memset(message,0,N_REQ);
        memset(host,0,N_HOST);
        memset(svc,0,N_HOST);
    }

    //Now we got the port
    // std::cout << "********\n";
    // std::cout << svc_user << " " << svc_peer << std::endl;
    while(1)
    {
        recv_message(message, sockfd, host,svc);
        if (std::string(host) == host_user && std::string(svc) == svc_user){
            send_message(host_peer.c_str(), svc_peer.c_str(), sockfd, message, strlen(message));
        }
        else if (std::string(host) == host_peer && std::string(svc) == svc_peer) {
            send_message(host_user.c_str(), svc_user.c_str(), sockfd, message, strlen(message));
        }
        else
        {
            perror("Unrecognized source 2");
            return;
        }
        memset(message,0,N_REQ);
        memset(host,0,N_HOST);
        memset(svc,0,N_HOST);
    }
    
}


void set_media_path(char* request, int& sockfd, int port, reg_table_type& register_table) {
    //start a new socket with a new port
    setup_sock(sockfd, port);
    std::string host_user, svc_user, host_peer, svc_peer, user_id,peer_id;
    parse_call_req(request,user_id,peer_id);
    
    if (register_table.find(user_id)!=register_table.end() &&
        register_table.find(peer_id)!=register_table.end()) {
        
        host_user = register_table[user_id][0];
        svc_user = register_table[user_id][1];
        
        host_peer = register_table[peer_id][0];
        svc_peer = register_table[peer_id][1];
        
        // std::cout << host_user << ":" << svc_user << "; " << host_peer << ":" << svc_peer <<std::endl;
        
        char message_front[N_REQ]={0};
        char message_back[N_REQ]={0};
        sprintf(message_front,"MEDIA_PORT FROM:%s TO:%s %d",peer_id.c_str(),user_id.c_str(),port);
        sprintf(message_back, "MEDIA_PORT FROM:%s TO:%s %d", user_id.c_str(),peer_id.c_str(),port);
 
        send_message(host_user.c_str(), svc_user.c_str(), sockfd, message_front, strlen(message_front));
        send_message(host_peer.c_str(), svc_peer.c_str(), sockfd, message_back, strlen(message_back));
        memset(message_front,0,N_REQ);
        memset(message_back,0,N_REQ);
        //start_session(sockfd,host_user,host_peer);
    }
    //exit(0);
    std::thread session(start_session,sockfd, host_user, host_peer);
    session.detach(); //crucial, so the proxy can actually be used for multiuser
}


int main(int argc, const char * argv[]) {
    char request[N_REQ]={0};
    char host[N_HOST]={0},svc[N_HOST]={0};
    int main_sockfd=0;
    int call_sock = 0;
    int call_port = 5000;
    reg_table_type register_table;
    
    
    setup_sock(main_sockfd,34567);
    
    
    while (1) {
        recv_message(request,main_sockfd,host,svc);

        if (req_type(request) == "REGISTER") {
            register_user(request,main_sockfd,register_table,host,svc);
            memset(request,0,N_REQ);
            memset(host,0,N_HOST);
            memset(svc,0,N_HOST);
        }
        else if(req_type(request) == "CALL"){
            process_call(request,main_sockfd,register_table);
            memset(request,0,N_REQ);
        }
        else if(req_type(request) == "ACK_CALL"){
            pass_call(request,main_sockfd,register_table);
            call_port++;
            set_media_path(request,call_sock,call_port,register_table);
            memset(request,0,N_REQ);
        }
        else if(req_type(request) == "CALL_FAILED"){
            pass_call(request,main_sockfd,register_table);
            memset(request,0,N_REQ);
        }
        else{
            perror("Unrecognized message");
        }
    }
    
    return 0;
}
