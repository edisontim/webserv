#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <map>
#include <string>

class Location
{
	private :
		std::map<std::string, std::string> location_rules;
		std::string url;
	public :
		Location();
		Location(std::string url);
		~Location(){};

		Location &operator=(Location const &cpy);
		std::map<std::string, std::string> &get_location_rules();
		std::string		get_url(void);
};

#endif
