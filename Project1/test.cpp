#include <iostream>
#include <bitset>


int main()
{ 
  std::bitset<8> b1;
  std::bitset<32> b2("10010011");
  std::cout << b2.test(0) << std::endl;
  std::cout << b2.test(1) << std::endl;
  std::cout << (b1==0) << std::endl;  
  std::cout << b2.to_string() << std::endl;
  std::cout << (b2<<8) << std::endl;
  std::cout << ((b2<<8)|b2) << std::endl;	
  std::cout << std::bitset<32>("010") << std::endl;
  std::string a,b;
  a="cav";
  b="ver";
  std::cout << a << b;
return 0;
}
