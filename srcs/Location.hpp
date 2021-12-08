#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <map>
#include <string>

class Location
{
	private :
		std::map<std::string, std::string> location_rules;
		std::string prefix;
	public :
		Location();
		Location(std::string prefix);
		~Location(){};

		Location &operator=(Location const &cpy);
		std::map<std::string, std::string> &get_location_rules();
		std::string		get_prefix(void);
};

#endif
