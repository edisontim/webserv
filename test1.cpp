#include <string>
#include <iostream>

int main()
{
	std::string s = "http://localhost:8080/?var1=10&name=tedi";
	std::string query_string;
	size_t find = s.rfind("?") + 1;
	if (find == 0)
		query_string = "";
	else
	{
		query_string = s.substr(find);
		s = s.substr(0, find - 1);
	}
	std::cout << "uri " << s << std::endl;
	std::cout << "query string " << query_string << std::endl;

}