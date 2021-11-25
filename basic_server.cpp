#include <iostream>

// NETWORKING SPECIFIC INCLUDES
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	struct sockaddr_in my_addr, other_addr;
	int connections_fd, actual_connection;
	int yes = 1;
	char message_buffer[255];

	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	my_addr.sin_port = htons(8080);
	// memset(my_addr.sin_zero, 0, sizeof my_addr.sin_zero);
	connections_fd = socket(PF_INET, SOCK_STREAM, 0);
	if(setsockopt(connections_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
	{
		std::cerr << "setsockopt : " << strerror(errno) << std::endl;
		return (1);
	}
	if (connections_fd == -1)
	{
		std::cerr << "socket : " << strerror(errno) << std::endl;
		return (1);
	}
	if (bind(connections_fd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1)
	{
		std::cerr << "bind : " << strerror(errno) << std::endl;
		return (1);
	}
	std::cout << "Before listen" << std::endl;
	if (listen(connections_fd, 5) == -1)
	{
		std::cerr << "listen : " << strerror(errno) << std::endl;
		return (1);
	}
	socklen_t size_addr = sizeof other_addr;
	actual_connection = accept(connections_fd, (struct sockaddr *) &other_addr, &size_addr);
	if (actual_connection == -1)
	{
		std::cerr << "accept : " << strerror(errno) << std::endl;
		return (1);
	}
	recv(actual_connection, message_buffer, 255, 0);
	std::cout << message_buffer << std::endl;
	std::cout << "Everything went well" << std::endl;
	return (0);
}