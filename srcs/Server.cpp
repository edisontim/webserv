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

Server::Server(std::string new_server_name, const char* hostname, const char* service) : sock(Internet_socket(hostname, service))
{
	struct pollfd new_pfd;

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

void Server::push_fd(int fd, int events)
{
	struct pollfd new_fd;

	new_fd.fd = fd;
	new_fd.events = events;
	pfds.push_back(new_fd);
}

void Server::pop_fd(void)
{
	pfds.pop_back();
}

//sends back number of connections including our socket fd
size_t Server::vector_size(void)
{
	return (pfds.size());
}

int Server::poll_fds(void)
{
	//buffer to hold the data we receive from an incoming connection
	char buff[9999];

	//buffer to hold the ip of our incoming connection
	char remote_ip[INET6_ADDRSTRLEN];

	//hold the strings from our request's parsing
	char *token[3];

	//struct of an incoming connection
	struct sockaddr_storage remoteaddr;
	socklen_t remoteaddr_len;

	//fd of our new connection
	int new_connection;

	size_type i;
	//poll our vector of fds
	int poll_count = poll(pfds.data(), pfds.size(), 3000);

	if (poll_count == -1)
	{
		std::cerr << "poll error" << std::endl;
		return(1);
	}

	//go through our array to check if one fd is ready to read
	i = 0;
	while (i < pfds.size())
	{
		if (pfds[i].revents & POLLIN) //one is ready
		{
			//our socket_fd is the one ready to read, this means a new incoming connection
			if (pfds[i].fd == sock.get_socket_fd())
			{
				remoteaddr_len = sizeof(remoteaddr);

				//accept the connection
				new_connection = accept(sock.get_socket_fd(), (struct sockaddr *)&remoteaddr, &remoteaddr_len);
				if (new_connection == -1)
					std::cerr << "error on accepting new connection" << std::endl;
				else
				{
					push_fd(new_connection, POLLIN | POLLOUT);
					std::cout << "new connection from " << inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remote_ip, INET6_ADDRSTRLEN) << std::endl;
				}
			}
			else //means that this is not out socket_fd, so this is a normal connection being ready to be read, so an http request is there
			{
				int nbytes = recv(pfds[i].fd, buff, sizeof(buff), 0);
				//error checking for recv (connection closed or error)
				if (nbytes <= 0)
				{
					if (nbytes == 0) //connection closed
					{
						std::cout << "Connection closed by client at socket " << pfds[i].fd << std::endl;
					}
					if (nbytes < 0)
						std::cerr << "recv error" << std::endl;
					close(pfds[i].fd);
					pfds.erase(pfds.begin() + i);
					continue ;
				}

				//no error was detected so the data received is valid

				//this is normally the first word of our request. This means the type : GET, POST, DELETE
				token[0] = strtok(buff, " \t\n");

				//this is the requested page of our request !!
				token[1] = strtok(NULL, " \t\n");

				//HTTP/1.1 or HTTP/1.0 if the request is valid
				token[2] = strtok(NULL, "\t\n\r");

				if (!token[0] || !token[1] || !token[2])
					continue ;
				
				//we are getting a GET request on server
				if (!strcmp(token[0], "GET"))
				{
					//treating HTTP/1.1 request
					if (!strcmp(token[2], "HTTP/1.1"))
					{
						//path of files directory -- VARIABLE TO CHANGE AFTER PARSING OF CONF FILE FOR THE ROOT DIRECTIVE
						const char path[] = "./website";
						
						std::string full_path = path;
						if (!strcmp(token[1], "/"))
							full_path += "/index.html";
						else
							full_path += token[1];
						
						//check if file exists, if it doesn't we need to send back the correct http response
						FILE *file_fd = fopen(full_path.c_str(), "r");
						std::string http_response;
						if (file_fd)
							http_response = response(full_path, token[2] , 200);
						else //404 page not found, fopen didn't find the page requested. Change the 404.hmtl by the correct default error page coming from the conf file
							http_response = response(std::string(path) + "/404.html", token[2], 404);
						if (pfds[i].revents & POLLOUT)
						{
							int bytes_sent = send(pfds[i].fd, http_response.c_str(), http_response.length(), 0);
							//number of bytes send differs from the size of the string, that means we had a problem with send()
							if (bytes_sent == -1 || bytes_sent != static_cast<int>(http_response.length()))
							{
								if (bytes_sent == -1)
									std::cerr << "error on send" << std::endl;
								else
									std::cerr << "send didn't write all the package" << std::endl;
								shutdown(pfds[i].fd, SHUT_RDWR);
								close(pfds[i].fd);
								pfds.erase(pfds.begin() + i);
							}
							//not sure these are necessary if recv and send worked
							// shutdown(pfds[i].fd, SHUT_RDWR);
							// close(pfds[i].fd);
							// pfds.erase(pfds.begin() + i);
						}
					}
				}
			}
		}
		i++;
	}
	return (0);
}
