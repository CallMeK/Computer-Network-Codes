#include <sstream>
#include <string>
#include <iostream>

int main()
{
	std::string s("AVRAEE rfr1234d AVREBE");
	std::stringstream ss(s);
	int value;
	char junk;
	std::string temp;
	ss >> temp;
	ss >> value;
	std::cout << value;
}
