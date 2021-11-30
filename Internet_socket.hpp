#ifndef INTERNET_SOCKET_HPP
#define INTERNET_SOCKET_HPP

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
		~Internet_socket();

		struct addrinfo get_hints(void);
		int get_socket_fd(void);
		std::string get_service(void);
};

#endif
