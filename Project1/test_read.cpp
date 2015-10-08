#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main()
{
  std::ifstream f("routes.txt");
  std::string temp;
  std::vector<std::string> result;
  while(std::getline(f,temp))
{
 result.push_back(temp);
 std::cout << temp <<std::endl;
}
 std::cout << result[0];
  std::stringstream ss(result[0]),ssl;
  std::string ip,n1,n2,n3,n4;
  int ip_size;
  char junk;	  
  ss >> ip;
  std::cout  << "ip: " << ip << std::endl;
  ssl << ip;
  ssl >> ip >> junk >> ip >> junk >> ip >> junk >> ip >> ip_size;
 std::cout << "****\n";
 std::cout << ip << ip_size <<  std::endl;

}
