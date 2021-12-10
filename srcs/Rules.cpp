#include "Rules.hpp"

Rules::Rules()
{
	directives.insert(std::make_pair("listen", ""));
	directives.insert(std::make_pair("server_name", ""));
	directives.insert(std::make_pair("error_page", ""));
	directives.insert(std::make_pair("client_max_body_size", ""));
	//set to off by default
	directives.insert(std::make_pair("autoindex", "off"));
	//set to executable's default directory
	directives.insert(std::make_pair("root", "."));
}

Rules::Rules(Rules const &cpy)
{
	*this = cpy;
}

Rules &Rules::operator=(Rules const &cpy)
{
	directives = cpy.directives;
	locations = cpy.locations;
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
