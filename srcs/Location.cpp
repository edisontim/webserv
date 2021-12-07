#include "Location.hpp"

Location::Location()
{
	location_rules.insert(std::make_pair("root", ""));
	location_rules.insert(std::make_pair("autoindex", ""));
	location_rules.insert(std::make_pair("root", ""));
	location_rules.insert(std::make_pair("GET", ""));
	location_rules.insert(std::make_pair("POST", ""));
	location_rules.insert(std::make_pair("DELETE", ""));
	location_rules.insert(std::make_pair("index", ""));
	location_rules.insert(std::make_pair("return", ""));
}

Location::Location(std::string new_url)
{
	url = new_url;
	
	location_rules.insert(std::make_pair("root", ""));
	location_rules.insert(std::make_pair("autoindex", ""));
	location_rules.insert(std::make_pair("root", ""));
	location_rules.insert(std::make_pair("GET", ""));
	location_rules.insert(std::make_pair("POST", ""));
	location_rules.insert(std::make_pair("DELETE", ""));
	location_rules.insert(std::make_pair("index", ""));
	location_rules.insert(std::make_pair("return", ""));
}


Location &Location::operator=(Location const &cpy)
{
	if (this != &cpy)
	{
		location_rules = cpy.location_rules;
	}
	return (*this);
}

std::map<std::string, std::string> &Location::get_location_rules()
{
	return (location_rules);
}

std::string	Location::get_url(void)
{
	return (url);
}
