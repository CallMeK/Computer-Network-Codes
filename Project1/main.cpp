//
//  main.cpp
//  Project1
//
//  Created by WuKe on 2/24/15.
//  Copyright (c) 2015 WuKe. All rights reserved.
//

#include "RoutesAnalyser.h"
#include <iostream>

void route_and_report(RoutesAnalyser& analyser, std::string& source, std::string& source_port, std::string& destination, std::string& dest_port, ARPtable& arp, int& ttl, std::ostream& output, Nattable& nat)
{

    ttl = ttl-1;
    std::string trans_source = source;
    std::string trans_source_port = source_port;
    std::string network, gateway, interface;
    std::vector<std::string> info;
    
    
    if (nat.check())
    {
        // Only when the nat.txt has something in it
        if (nat.tablecheck(destination,dest_port))
        {
            network = analyser.search(destination);
            info = analyser.report(network);
            gateway = info[0];
            interface = info[1];
            nat.translate_forward(interface, source, source_port, trans_source, trans_source_port);
        }
        else
        {
            // The destination is in the table
            nat.translate_back(destination,dest_port);
            network = analyser.search(destination);
            info = analyser.report(network);
            gateway = info[0];
            interface = info[1];
        }
    }
    else
    {
    
        /* On routing table*/
        network = analyser.search(destination);
        info = analyser.report(network);
        gateway = info[0];
        interface = info[1];
    }
    /* The report part*/
    
    output << trans_source << ":" << trans_source_port << "->" << destination << ":" << dest_port << " ";
    if (ttl <= 0) {
        output << "discarded (TTL expired)\n";
    }
    else if(gateway=="0.0.0.0")
    {
        gateway = destination;
        std::string mac = arp.search(gateway); //since it is local, should always find the mac
        output << "directly connected " << "(" << interface << "-" << mac << ")" << " ttl " << ttl << std::endl;
    }
    else
    {

        if (interface.substr(0,3) == "ppp" ) {
            output << "via " << gateway << " (" << interface << ") ttl " << ttl << std::endl;
        }
        else{
            std::string mac = arp.search(gateway);
            if (mac == "Not Found") {
                output << "discarded (destination unreachable)\n";
            }
            else{
                output << "via " << gateway << "(" << interface << "-" << mac << ")" << " ttl " << ttl <<std::endl;
            }
        }
    }
}





int main(int argc, const char * argv[]) {
    std::string interface,source,destination,protocol_num,source_port,dest_port;
    int ttl;
    RoutesAnalyser analyser(argv[1]);
    ARPtable arp(argv[2]);
    Nattable nat(argv[3]);
    std::ofstream output("output.txt");
    while (std::cin >> interface >> source >> destination >> protocol_num >> ttl >> source_port >> dest_port )
    {
        route_and_report(analyser,source,source_port,destination,dest_port,arp,ttl,std::cout,nat);

    }
   
    
    return 0;
}
