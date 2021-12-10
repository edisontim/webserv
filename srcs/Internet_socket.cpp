#include "Internet_socket.hpp"

Internet_socket::Internet_socket()
{
	
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

void Internet_socket::display_IP()
{
	void *addr;
	addr = &(ip_port.sin_addr);
	char addr_str[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET,  addr, addr_str, sizeof(addr_str));
	std::cout << "IP : " << addr_str << std::endl;
}

int Internet_socket::bind_listen(const char* hostname, const char *service)
{
	struct addrinfo *res;
	struct addrinfo *iter;
	int yes = 1;

	_service = service;

	bzero(&hints, sizeof(hints));
	
	//Take both IPv4 or IPv6
	hints.ai_family = AF_UNSPEC;

	//get a stream socket
	hints.ai_socktype = SOCK_STREAM;

	//hints.ai_protocol is set to 0 by bzero, so the protocol is "any" that fits the other criterias
	
	//get my IP automatically if the ip wasn't specified
	if (!hostname)
		hints.ai_flags = AI_PASSIVE;

	//stores a list of potentially usable addresses in res, we need to iterate through it
	//to find a valid address to create a socket and bind that socket
	getaddrinfo(hostname, service, &hints, &res);

	//iterate through all the addresses resulting from the call to getaddrinfo 
	//to try to get a socket a	nd bind it
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

	if (iter)
	{
		//display address bound to the socket
		if (iter->ai_family == AF_INET)
		{
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)iter->ai_addr;
			void *addr;
			addr = &(ipv4->sin_addr);
			char addr_str[INET6_ADDRSTRLEN];
			inet_ntop(iter->ai_family,  addr, addr_str, sizeof(addr_str));
			std::cout << "IP : " << addr_str << std::endl;
			std::cout << "Port : " << service << std::endl;
		}
		else
		{
			std::cout << "IPv6 motherfucker" << std::endl;
		}
	}
	else
		std::cout << "No socket was connected" << std::endl;
	
	ip_port = *(struct sockaddr_in *)iter->ai_addr;
	freeaddrinfo(res);

	// if we got to the end of the linked list, that means no socket_fd was binded
	if (iter == NULL)
	{
		socket_fd = NO_BOUND;
		return (NO_BOUND);
	}


	if (listen(socket_fd, 10) == -1)
	{
		socket_fd = LISTEN_FAIL;
		return (LISTEN_FAIL);
	}
	return (0);


	// (void)service;

	// int _true;
	// struct sockaddr_in serv_addr;
	// int sock_fd;

	// std::cout << "hostname : " << hostname << std::endl;
	// std::cout << "service : " << service << std::endl;
	// serv_addr.sin_family = AF_INET;
	// serv_addr.sin_port = htons(8080);
	// serv_addr.sin_addr.s_addr = inet_addr(hostname);
	
	// memset(serv_addr.sin_zero, '\0', sizeof(serv_addr.sin_zero));
	
	// sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	
	
	// setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &_true, sizeof(_true));
	
	// if (sock_fd < 0)
	// 	std::cerr << "fail with socket" << std::endl;

	// if (bind(sock_fd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
	// 	std::cerr << "fail with bind" << std::endl;

	// socket_fd = sock_fd;

	// if (listen(sock_fd, 5) == -1)
	// 	return (LISTEN_FAIL);
	// return (1);
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
