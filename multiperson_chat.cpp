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

#include "Internet_socket.hpp"

void *get_in_addr(struct sockaddr *address)
{
	if (address->sa_family == AF_INET)
		return &(((struct sockaddr_in *)address)->sin_addr);
	else
		return &(((struct sockaddr_in6 *)address)->sin6_addr);
}


int main()
{
	std::vector<struct pollfd> pfds;
	Internet_socket bound_sock;
	struct pollfd new_pfd; //pfd is either our bound socket or a remote connection
	int i;
	int new_connection;
	struct sockaddr_storage remoteaddr;
	socklen_t remoteaddr_len;
	char remotedata_buff[256];
	char remote_ip[INET6_ADDRSTRLEN];


	if (bound_sock.get_socket_fd() < 0)
	{
		std::cout << "error with the listening socket : " << bound_sock.get_socket_fd() << std::endl;
		return (1);
	}

	//get and print owns IPv4 address
	{
		char ip_string[255];
		struct hostent *host;

		//this gets the official name of the host machine running the program
		gethostname(ip_string, INET6_ADDRSTRLEN);

		// sends back a pointer to hostent struct, by casting the h_addr (which is a define to the first element of the h_addr_list) to an in_addr* and dereferencing that we can get an ip addr in the form of a in_addr
		// this is a valid method but I think getaddrinfo with hints.ai_flags set to AI_PASSIVE to just use our own IP is easier
		host = gethostbyname(ip_string);
		struct sockaddr_in *addr = (struct sockaddr_in *)bound_sock.get_hints().ai_addr;
		std::cout << "Users can connect to <" << inet_ntoa(*((struct in_addr *)host->h_addr)) << "> on port <" << bound_sock.get_service() << ">"<< std::endl;
	}

	new_pfd.fd = bound_sock.get_socket_fd();
	new_pfd.events = POLLIN;
	pfds.push_back(new_pfd);

	//check until program is quit for an fd in our vector that has been set to POLLIN (ready to read)
	while (1)
	{
		//poll our vector of fds
		int poll_count = poll(pfds.data(), pfds.size(), -1);
		if (poll_count == -1)
		{
			std::cerr << "poll error" << std::endl;
			return(1);
		}
		i = 0;

		//go through our array to check if one fd is ready to read
		while (i < pfds.size())
		{
			if (pfds[i].revents & POLLIN) //one is ready
			{
				//our socket_fd is the one ready to read, this means a new incoming connection
				if (pfds[i].fd == bound_sock.get_socket_fd())
				{
					remoteaddr_len = sizeof(remoteaddr);
					new_connection = accept(bound_sock.get_socket_fd(), (struct sockaddr *)&remoteaddr, &remoteaddr_len);
					if (new_connection == -1)
						std::cerr << "error on accepting new connection" << std::endl;
					else
					{
						new_pfd.fd = new_connection;
						new_pfd.events = POLLIN;
						pfds.push_back(new_pfd);
						std::cout << "new connection from " << inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remote_ip, INET6_ADDRSTRLEN) << std::endl;
					}
				}
				else //means that this is not out socket_fd, so this is a normal connection being ready to be read, ergo someone typed a message and sent it
				{
					int nbytes = recv(pfds[i].fd, remotedata_buff, sizeof(remotedata_buff), 0);
					int sender_fd = pfds[i].fd;

					if (nbytes <= 0) //connection was closed or got an error
					{
						if (nbytes == 0) //connection closed
							std::cerr << "pollserver : socket " << sender_fd << " hung up" << std::endl;
						else
							std::cerr << "recv error" << std::endl;
						close(pfds[i].fd); //close fd
						pfds.erase(pfds.begin() + i); //erase the pfd from our vector
					}
					else // data is good and readable
					{
						int j = 0;
						while (j < pfds.size())
						{
							int dest_fd = pfds[j].fd;
							if (dest_fd != pfds[0].fd && dest_fd != sender_fd)
							{
								if (send(dest_fd, remotedata_buff, nbytes, 0) == -1)
								{
									std::cerr << "error with send" << std::endl;
								}
							}
							j++;
						}
					}
				}
			}
			i++;
		}
	}
	return (0);
}