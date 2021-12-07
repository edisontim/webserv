#include "Rules.hpp"

Rules::Rules()
{
	directives.insert(std::make_pair("listen", ""));
	directives.insert(std::make_pair("server_name", ""));
	directives.insert(std::make_pair("error_page", ""));
	directives.insert(std::make_pair("client_max_body_size", ""));
	directives.insert(std::make_pair("root", ""));
}

Rules &Rules::operator=(Rules const &cpy)
{
	directives = cpy.directives;
	return (*this);
}

std::map<std::string, std::string> &Rules::get_directives(void)
{
	return (directives);
}

//returns our vector of location directives
std::vector<Location> &Rules::get_locations(void)
{
	return (locations);
}
