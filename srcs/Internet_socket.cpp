#include "Internet_socket.hpp"

Internet_socket::Internet_socket(const char* hostname, const char *service) : _service(service)
{
	struct addrinfo *res;
	struct addrinfo *iter;
	int yes = 1;

	bzero(&hints, sizeof(hints));
	
	//Take both IPv4 or IPv6
	hints.ai_family = AF_UNSPEC;

	//stream socket
	hints.ai_socktype = SOCK_STREAM;
	
	//get my IP automatically
	hints.ai_flags = AI_PASSIVE;
	
	//hints.ai_protocol is set to 0 by bzero, so the protocol is "any" that fits the other criterias
	getaddrinfo(hostname, service, &hints, &res);

	//iterate through all the addresses resulting from the call to getaddrinfo 
	//to try to get a socket and bind it
	for (iter = res; iter != NULL; iter = iter->ai_next)
	{
		socket_fd = socket(iter->ai_family, iter->ai_socktype, hints.ai_protocol);

		if (socket_fd < 0)
			continue;

		// Stop the "address already in use" error message
		setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

		if (bind(socket_fd, iter->ai_addr, iter->ai_addrlen) < 0)
		{
			close(socket_fd);
			continue;
		}
		break;
	}

	// if we got to the end of the linked list, that means no socket_fd was binded
	if (iter == NULL)
		socket_fd = NO_BOUND;

	freeaddrinfo(res);

	if (listen(socket_fd, 10) == -1)
		socket_fd = LISTEN_FAIL;
}

Internet_socket::Internet_socket(Internet_socket const &cpy)
{
	*this = cpy;
}

Internet_socket::~Internet_socket()
{
	if (socket_fd > -1)
		close(socket_fd);
}


struct addrinfo Internet_socket::get_hints(void)
{
	return (hints);
}

int Internet_socket::get_socket_fd(void)
{
	return (socket_fd);
}

std::string Internet_socket::get_service(void)
{
	return (_service);
}


Internet_socket &Internet_socket::operator=(Internet_socket const &cpy)
{
	if (this != &cpy)
	{
		hints = cpy.hints;
		socket_fd = cpy.socket_fd;
		_service = cpy._service;
	}
	return (*this);
}
