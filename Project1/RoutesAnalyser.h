//
//  RoutesAnalyser.h
//  Project1
//
//  Created by WuKe on 2/25/15.
//  Copyright (c) 2015 WuKe. All rights reserved.
//

#ifndef __Project1__RoutesAnalyser__
#define __Project1__RoutesAnalyser__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <bitset>
#include <map>
#include <set>

class node
{
public:
    std::bitset<32> network_prefix;
    std::string ip_str;
    node* left;
    node* right;
    
    node()
    {
        this->left = NULL;
        this->right = NULL;
    }
    node(std::bitset<32> network_prefix_)
    {
        this->network_prefix = network_prefix_;
        this->left = NULL;
        this->right = NULL;
    }
    void SetValue(std::bitset<32> network_prefix_)
    {
        this->network_prefix = network_prefix_;
    }
    ~node()
    {
        delete this->left;
        delete this->right;
    }
};

class BinaryTrie
{
public:
    BinaryTrie()
    {
        this->head = NULL;
    }
    BinaryTrie(std::vector<std::string>& routes_dest)
    {
        this->set(routes_dest);
    }
    void set(std::vector<std::string>& routes_dest)
    {
        for(unsigned int i = 0; i < routes_dest.size(); i++)
        {
            this->insert(routes_dest[i]);
        }
    }
    ~BinaryTrie()
    {
        delete this->head;
    }
    const std::string& find(const std::string& new_ip);
private:
    void ip_convert(const std::string& network_prefix, std::bitset<32>& temp, int& size);
    void IptoBitset(const std::string& network_prefix, std::bitset<32>& temp);
    void insert(const std::string& network);
    
    node* head;
    
};


class RoutesAnalyser
{
public:
    RoutesAnalyser(std::string filename)
    {
        this->filename = filename;
        createTable();
        createBinaryTrie();
    }
    std::string search(const std::string& new_ip);
    const std::vector<std::string>& report(const std::string& matched_network)
    {
        return table[matched_network];
    }
    void printTable();
private:
    void createTable();
    void createBinaryTrie();
    std::map<std::string, std::vector<std::string> > table;
    std::string filename;
    BinaryTrie bt;
};


class ARPtable
{
public:
    ARPtable(const std::string& filename)
    {
        this->filename = filename;
        this->err = "Not Found";
        createTable();
    }
    const std::string& search(const std::string& ip)
    {
        if (this->table.find(ip)!=this->table.end())
        {
            return this->table[ip];
        }
        else
        {
            return this->err;
        }
    }
private:
    void createTable();
    std::map<std::string,std::string> table;
    std::string filename;
    std::string err;
};


class Nattable
{
public:
    Nattable(const std::string& filename)
    {
        this->filename = filename;
        this->err = "Not Found";
        createTable();
    }
    const std::string& search(const std::string& interface)
    {
        if (this->table.find(interface)!=this->table.end())
        {
            return this->table[interface];
        }
        else
        {
            return this->err;
        }
    }
    bool check()
    {
        return this->exist;
    }
    void translate_forward(std::string& interface,std::string& source, std::string& source_port, std::string& trans_source, std::string& trans_source_port);
    void translate_back(std::string& destination, std::string& dest_port);
    bool tablecheck(std::string& destination, std::string& dest_port);
    void printTranstable();
private:
    void createTable();
    std::map<std::string, std::string> table;
    std::string filename;
    std::string err;
    std::map<std::string, std::string> transtable;
    std::set<std::string> portset;
    bool exist;
};




#endif /* defined(__Project1__RoutesAnalyser__) */
