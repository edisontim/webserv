#ifndef RULES_HPP
#define RULES_HPP

#include <iostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <ctime>
#include <sstream>
#include <map>

class Rules
{
	private:
		std::map<std::string, std::string> directives;
		//add setup of routes

	public:
		Rules();
		~Rules(){};

		Rules &operator=(Rules const &cpy);
		std::map<std::string, std::string> &get_directives(void);
};

#endif
