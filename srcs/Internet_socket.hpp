#ifndef INTERNET_SOCKET_HPP
#define INTERNET_SOCKET_HPP

#include "webserv.hpp"

#define NO_BOUND -1
#define LISTEN_FAIL -2

class Internet_socket
{
	private:
		struct addrinfo	hints;
		int				socket_fd;
		std::string		_service;

	public:
		Internet_socket(const char* hostname = NULL, const char *service = "80");
		Internet_socket(Internet_socket const &cpy);
		~Internet_socket();

//		___________OPERATORS___________

		Internet_socket &operator=(Internet_socket const &cpy);


//		___________GETTER/SETTERS___________

		struct addrinfo	get_hints(void);
		int				get_socket_fd(void);
		std::string		get_service(void);
};

#endif
