#include "Server.hpp"

Server::Server(Internet_socket const &socket_fd, std::string new_server_name)
{
	struct pollfd new_pfd;

	sock = socket_fd;
	server_name = new_server_name;
	
	new_pfd.fd = sock.get_socket_fd();
	new_pfd.events = POLLIN;
	pfds.push_back(new_pfd);
}

Server::Server(std::string new_server_name, const char* hostname, const char* service)
{
	struct pollfd new_pfd;

	sock = Internet_socket(hostname, service);
	server_name = new_server_name;
	
	new_pfd.fd = sock.get_socket_fd();
	new_pfd.events = POLLIN;
	pfds.push_back(new_pfd);
}

Internet_socket &Server::get_sock(void)
{
	return (sock);
}

std::string &Server::get_server_name(void)
{
	return (server_name);
}

std::vector<struct pollfd> &Server::get_pfds(void)
{
	return (pfds);
}

void Server::push_fd(struct pollfd new_fd)
{
	pfds.push_back(new_fd);
}

void Server::pop_fd(void)
{
	pfds.pop_back();
}

size_t Server::vector_size(void)
{
	return (pfds.size());
}