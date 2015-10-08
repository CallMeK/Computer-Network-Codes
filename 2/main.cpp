//
//  main.cpp
//  HW2
//
//  Created by WuKe on 3/13/15.
//  Copyright (c) 2015 WuKe. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <thread>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <chrono>

#define BUFSIZE 256

void send_message(int sockfd, const char* message, size_t size_message)
{
    long ret;
    struct sockaddr_in dest;
    
    dest.sin_family = AF_INET;
    dest.sin_port = htons(23456);
    if (inet_aton("239.255.24.25", &dest.sin_addr) == 0) {
        fprintf(stderr, "invalid address!\n");
        return;
    }
    
    while (1) {
        ret = sendto(
                 sockfd, message, size_message, 0,
                 (struct sockaddr *)&dest, sizeof(dest)
                 );
        if (ret == -1) {
            perror("sendto");
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(25));
    }
}

void send_message_mult(int sockfd, const char* message, size_t size_message)
{
    long ret;
    struct sockaddr_in dest;
    
    dest.sin_family = AF_INET;
    dest.sin_port = htons(23456);
    if (inet_aton("239.255.24.25", &dest.sin_addr) == 0) {
        fprintf(stderr, "invalid address!\n");
        return;
    }
    
        ret = sendto(
                     sockfd, message, size_message, 0,
                     (struct sockaddr *)&dest, sizeof(dest)
                     );
        if (ret == -1) {
            perror("sendto");
            return;
        }
}



void send_message_to_person(int sockfd, std::string message, std::string name, std::map<std::string, std::vector<std::string> >& addr_table)
{
    
    long ret;
    struct sockaddr_in dest;
    
    if (addr_table.find(name) != addr_table.end())
    {
        //there is such record
        std::string ip = addr_table[name][0];
        std::string port = addr_table[name][1];
        
        
        dest.sin_family = AF_INET;
        dest.sin_port = htons(std::stoi(port));
        if (inet_aton(ip.c_str(), &dest.sin_addr) == 0) {
            fprintf(stderr, "invalid address!\n");
            return;
        }
    
        ret = sendto(
                 sockfd, message.c_str(), message.size(), 0,
                 (struct sockaddr *)&dest, sizeof(dest)
                 );
        if (ret == -1) {
            perror("sendto");
            return;
        }
    }
    else
    {
        std::cout << "Person Not Found\n";
    }
    
}


void recv_any(int sockfd, std::map<std::string, std::vector<std::string> >& addr_table)
{
    struct sockaddr_in src;
    socklen_t srclen;
    char buf[BUFSIZE];
    ssize_t size;
    int ret;
    
    struct ip_mreq mreq;
    
    srclen = sizeof(src);
    
    /* subscribe to multicasts to the address of interest */
    
    const char *multicast_addr = "239.255.24.25";
    
    if (inet_pton(AF_INET, multicast_addr, &mreq.imr_multiaddr) != 1) {
        fprintf(stderr, "inet_pton failed\n");
        return;
    }
    
    mreq.imr_interface.s_addr = INADDR_ANY;
    
    ret = setsockopt(
                     sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                     &mreq, sizeof(mreq)
                     );
    
    if (ret != 0) {
        perror("setsockopt");
        return;
    }

    
    while (1)
    {
        size = recvfrom(
                    sockfd, buf, BUFSIZE, 0,
                    (struct sockaddr *)&src, &srclen
                    );
        std::string message(buf);
        if (message.substr(0,8)=="ANNOUNCE") {
            //It is an announcement
            char host[80],svc[80];
            ret = getnameinfo(
                              (struct sockaddr *)&src, sizeof(src),
                              host, sizeof(host),
                              svc, sizeof(svc),
                              0 | NI_NUMERICHOST
                              );
            
            if (ret != 0) {
                fprintf(
                        stderr, "getnameinfo() failed: %s\n", 
                        gai_strerror(ret)
                        );
            }
            else{
                //printf("received from %s (%s):\n", host, svc);
                std::string ip(host);
                std::string port(svc);
                std::string name(message.substr(9));
                addr_table[name].push_back(ip);
                addr_table[name].push_back(port);
            }
        }
        else
        {
            printf("%s\n",message.c_str());
        }
        memset(buf,0,sizeof(char)*BUFSIZE);
    }
 
}


int main(int argc, const char * argv[]) {
    
    //use only one socket
    //bind the socket
    
    int sockfd;
    struct sockaddr_in bindaddr;
    
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_port = htons(23456);
    bindaddr.sin_addr.s_addr = INADDR_ANY;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return 1;
    }
    
    if (bind(sockfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) != 0) {
        perror("bind");
        return 1;
    }
    
    
    std::map<std::string,std::vector<std::string> > addr_table;
    
    std::string username(argv[1]);
    std::string ANNOUNCE("ANNOUNCE ");
    
    std::thread thread_send_message(send_message,sockfd, (ANNOUNCE+username).c_str(),(ANNOUNCE+username).size());
    std::thread thread_recv_any(recv_any,sockfd, std::ref(addr_table));
    
    
    // a for loop to make sure that multiple messages can be sent
    while (1)
    {
        std::string temp;
        bool first_argument = true;
        bool multicast=true;
        std::string name_to_send;
        std::string messages_to_send;
        messages_to_send.clear();
        while   (std::cin.get()!='\n')
        {
            std::cin.unget();
            std::cin >> temp;
            if (first_argument) {
                first_argument = false;
                if(temp.c_str()[0] == '/' ) {
                    multicast = false;
                    name_to_send = temp.substr(1); // get rid of the '/'
                }
                else
                {
                    messages_to_send =messages_to_send+temp + " ";
                }
            }
            else{
                messages_to_send =messages_to_send+temp + " ";
            }
            
        }
        messages_to_send = messages_to_send.substr(0,messages_to_send.size()-1); //get rif of the final space
        if (multicast) {
            std::string PREFIX("From:");
            std::string messages_to_send_mult = PREFIX+argv[1]+" "+messages_to_send;
            send_message_mult(sockfd, messages_to_send_mult.c_str(), messages_to_send_mult.size());
        }
        else{
            send_message_to_person(sockfd, messages_to_send.c_str(), name_to_send,addr_table);
        }
        std::cin.clear();
        std::cin.sync();
    }
    //thread_send_message.join();
    //thread_recv_any.join();
    
}
