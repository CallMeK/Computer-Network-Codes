//
//  RoutesAnalyser.cpp
//  Project1
//
//  Created by WuKe on 2/25/15.
//  Copyright (c) 2015 WuKe. All rights reserved.
//

#include "RoutesAnalyser.h"


void BinaryTrie::ip_convert(const std::string& network_prefix, std::bitset<32>& temp, int& size)
{
    std::istringstream ss(network_prefix);
    int n1,n2,n3,n4;
    char junk;
    ss >> n1 >> junk >> n2 >> junk >> n3 >> junk >> n4 >> junk >> size;
    std::bitset<32> v1(n1);
    std::bitset<32> v2(n2);
    std::bitset<32> v3(n3);
    std::bitset<32> v4(n4);
    temp = (v1<<24)|(v2<<16)|(v3<<8)|(v4);
}

void BinaryTrie::IptoBitset(const std::string &network_prefix, std::bitset<32> &temp)
{
    std::istringstream ss(network_prefix);
    int n1,n2,n3,n4;
    char junk;
    ss >> n1 >> junk >> n2 >> junk >> n3 >> junk >> n4;
    std::bitset<32> v1(n1);
    std::bitset<32> v2(n2);
    std::bitset<32> v3(n3);
    std::bitset<32> v4(n4);
    temp = (v1<<24)|(v2<<16)|(v3<<8)|(v4);
}

void BinaryTrie::insert(const std::string &network)
{
    //the network_prefix is always 32 bits, using the size_ to determine the length
    //which means, for 1.0.0.0/8 it will only be stored as 00000001 with 24 0's, it is also
    //how the rounting table is actually stored
    //It is hard to change the bitset dynamically...
    
    int size_;
    std::bitset<32> network_prefix;
    this->ip_convert(network, network_prefix, size_);
    if (!this->head) {
        // The trie has not been built
        this->head = new node();
    }
    node* temp_ptr;
    temp_ptr = this->head;
    for (int i=31; i>(31-size_); i--)
    {
        if(network_prefix.test(i))
        {
            if (!temp_ptr->right)
            {
                temp_ptr->right = new node();
            }
            temp_ptr = temp_ptr->right;
        }
        else
        {
            if (!temp_ptr->left)
            {
                temp_ptr->left = new node();
            }
            temp_ptr = temp_ptr->left;
        }
    }
    temp_ptr->network_prefix = network_prefix;
    temp_ptr->ip_str = network;
    
}

const std::string& BinaryTrie::find(const std::string& new_ip_str)
{
    std::bitset<32> new_ip;
    this->IptoBitset(new_ip_str, new_ip);
    node* result;
    node* ptr=this->head;
    result = ptr;
    if (ptr)
    {
        for (int i=31; i>=0; i--)
        {
            if (new_ip.test(i)) {
                ptr = ptr->right;
                //std::cout << "right\n";
            }
            else
            {
                ptr = ptr->left;
                //std::cout << "left\n";
                
            }
            if (ptr) {
                //std::cout << ptr->network_prefix << std::endl;
                if (ptr->network_prefix!=0)
                {
                    result = ptr;
                }
            }
            else
                break;
            
        }
    }
    else
    {
        std::cerr << "The trie has not been built yet";
    }
    return result->ip_str;
}

std::string RoutesAnalyser::search(const std::string& new_ip)
{
    std::string result;
    result = this->bt.find(new_ip);
    return result;
}

void RoutesAnalyser::createTable()
{
    std::string key_temp, temp;
    std::vector<std::string> val_temp;
    std::string line_temp;
    std::ifstream f(this->filename);
    if(f.fail()) std::cerr << "Failed to open the file\n";
    std::stringstream ss;
    
    while (std::getline(f, line_temp))
    {
        if(line_temp.size()!=0)
        {
            ss << line_temp;
            ss >> key_temp;
            while (ss >> temp)
            {
                val_temp.push_back(temp);
            }
            this->table[key_temp] = val_temp;
            ss.clear();
            val_temp.clear();
        }
    }
    f.close();
}


void RoutesAnalyser::printTable()
{
    for (std::map<std::string,std::vector<std::string> >::iterator i=this->table.begin(); i!=this->table.end(); i++) {
        std::cout << i->first << " ";
        for (unsigned int j=0;j<2;j++)
        {
            std::cout << i->second[j] << " ";
        }
        std::cout << std::endl;
    }
}

void RoutesAnalyser::createBinaryTrie()
{
    std::vector<std::string> routes_dest;
    for (std::map<std::string,std::vector<std::string> >::iterator i=this->table.begin(); i!=this->table.end(); i++)
    {
        routes_dest.push_back(i->first);
    }
    this->bt.set(routes_dest);
}

void ARPtable::createTable()
{
    std::string ip_temp;
    std::string mac_temp;
    std::string line_temp;
    std::ifstream f(this->filename);
    if(f.fail()) std::cerr << "Failed to open the file\n";
    std::stringstream ss;
    
    while (std::getline(f, line_temp))
    {
        if(line_temp.size()!=0)
        {
            ss << line_temp;
            ss >> ip_temp >> mac_temp ;
            this->table[ip_temp] = mac_temp;
            ss.clear();
        }
    }
    f.close();
}

void Nattable::createTable()
{
    std::string interface_temp, ip_temp, line_temp;
    std::ifstream f(this->filename);
    if(f.fail()) std::cerr << "Failed to open the nat file, if you are trying to test part1, use an empty nat.txt\n";
    std::stringstream ss;
    while (std::getline(f, line_temp))
    {
        if(line_temp.size()!=0)
        {
            ss << line_temp;
            ss >> interface_temp >> ip_temp ;
            this->table[interface_temp] = ip_temp;
            ss.clear();
        }
    }
    if (this->table.size()==0) {
        this->exist = false;
    }
    else
    {
        this->exist = true;
    }
    f.close();

}

void Nattable::translate_forward(std::string& interface,std::string& source, std::string& source_port, std::string& trans_source, std::string& trans_source_port)
{
    //only do the translation if the nat table exists
    //the format of the trantable
    // translated ip:port to original ip:port
    if (this->table.find(interface)!=this->table.end())
    {
    //There is such interface in the nat

           
        trans_source = this->table[interface];
        trans_source_port = source_port;
        int temp;
        std::stringstream buffer(trans_source_port);
        while  (this->portset.find(trans_source_port)!=this->portset.end())
        {
            buffer >> temp;
            temp++;
            buffer.clear();
            buffer << temp;
            trans_source_port = buffer.str();
        }
        this->transtable[trans_source+":"+trans_source_port] = source+":"+source_port;
        this->portset.insert(trans_source_port);
    }
}



void Nattable::translate_back(std::string& destination, std::string& dest_port)
{

    //this is an incoming package (response)
    //I directly changed the value, since there is no reason to record the original values
    std::string actual_ip;
    actual_ip = this->transtable[destination+":"+dest_port];
    destination = actual_ip.substr(0,actual_ip.find(":"));
    dest_port = actual_ip.substr(actual_ip.find(":"));
//    std::cout << destination << " " << dest_port << std::endl;
}



void Nattable::printTranstable()
{
    for (std::map<std::string, std::string>::iterator i=this->transtable.begin(); i!=this->transtable.end(); i++) {
        std::cout << i->first << " translated from " << i->second <<std::endl;
    }
}

bool Nattable::tablecheck(std::string &destination, std::string &dest_port)
{
    if(this->transtable.find(destination+":"+dest_port)==this->transtable.end())
    {
        return true;
    }
    else
        return false;
}



