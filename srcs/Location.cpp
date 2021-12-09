#include "Location.hpp"

Location::Location()
{
	//defaults to off
	location_rules.insert(std::make_pair("autoindex", "off"));
	//defaults to the default directory
	location_rules.insert(std::make_pair("root", "."));
	//if this is something else than true, it's automatically false
	location_rules.insert(std::make_pair("GET", "true"));
	location_rules.insert(std::make_pair("POST", "true"));
	location_rules.insert(std::make_pair("DELETE", "false"));
	//defaults to index.html
	location_rules.insert(std::make_pair("index", "index.html"));
	location_rules.insert(std::make_pair("return", ""));
	//defaults to 404.html
	location_rules.insert(std::make_pair("error_page", "404.html"));
}

Location::Location(std::string new_url)
{
	prefix = new_url;
	
	//defaults to off
	location_rules.insert(std::make_pair("autoindex", "off"));
	//defaults to the default directory
	location_rules.insert(std::make_pair("root", "."));
	//if this is something else than true, it's automatically false
	location_rules.insert(std::make_pair("GET", "true"));
	location_rules.insert(std::make_pair("POST", "true"));
	location_rules.insert(std::make_pair("DELETE", "false"));
	//defaults to index.html
	location_rules.insert(std::make_pair("index", "index.html"));
	location_rules.insert(std::make_pair("return", ""));
	//defaults to 404.html
	location_rules.insert(std::make_pair("error_page", "404.html"));
}


Location &Location::operator=(Location const &cpy)
{
	if (this != &cpy)
	{
		location_rules = cpy.location_rules;
		prefix = cpy.prefix;
	}
	return (*this);
}

std::map<std::string, std::string> &Location::get_location_rules()
{
	return (location_rules);
}

std::string	Location::get_prefix(void)
{
	return (prefix);
}
