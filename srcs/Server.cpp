#include "Server.hpp"

Server::Server(Internet_socket const &socket_fd)
{
	struct pollfd new_pfd;

	sock = socket_fd;

	new_pfd.fd = sock.get_socket_fd();
	new_pfd.events = POLLIN;
	pfds.push_back(new_pfd);
}

Server::Server(const char* service, const char* hostname)
{
	struct pollfd new_pfd;

	sock.bind_listen(hostname, service);

	new_pfd.fd = sock.get_socket_fd();
	new_pfd.events = POLLIN;
	pfds.push_back(new_pfd);
}

void	Server::display_IP(void)
{
	sock.display_IP();
}

Internet_socket &Server::get_sock(void)
{
	return (sock);
}

std::vector<Virtual_server> &Server::get_v_servers(void)
{
	return (v_servers);
}

std::vector<struct pollfd> &Server::get_pfds(void)
{
	return (pfds);
}

void Server::set_rules(Rules &new_rules)
{
	rule_set = new_rules;
}

Rules &Server::get_rules(void)
{
	return (rule_set);
}

void Server::push_v_server(Virtual_server new_server)
{
	v_servers.push_back(new_server);
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
	// char remote_ip[INET6_ADDRSTRLEN];

	//hold the strings from our request's parsing
	char *token[3];

	//struct of an incoming connection
	struct sockaddr_storage remoteaddr;
	socklen_t remoteaddr_len;

	//fd of our new connection
	int new_connection;

	size_type i;
	//poll our vector of fds
	int poll_count = poll(pfds.data(), pfds.size(), 0);

	if (poll_count == -1)
	{
		std::cerr << "poll error" << std::endl;
		return (0);
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
					// std::cout << "new connection from " << inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remote_ip, INET6_ADDRSTRLEN) << std::endl;
				}
			}
			else //means that this is not out socket_fd, so this is a normal connection being ready to be read, so an http request is there
			{
				//loop here while nbytes > 0 ?
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

				//this is the requested page of our request /upload/lo.html
				token[1] = strtok(NULL, " \t\n");

				//HTTP/1.1 or HTTP/1.0 if the request is valid
				token[2] = strtok(NULL, "\t\n\r");

				if (!token[0] || !token[1] || !token[2])
					continue ;
				
				if (!strcmp(token[0], "POST"))
					std::cout << "Got POST request" << std::endl;
				
				// We need to parse the request to get the hostname!!!
				std::string hostname = "127.0.0.1:8080";

				std::string http_response = "";
				for (unsigned int j = 0; j < this->get_v_servers().size(); j++)
				{
					//if we find a virtual server whose server_name directives matches with the Host field
					// of our request, that's the one that should treat it
					if (!get_v_servers()[j].get_rules().get_directives()["server_name"].compare(hostname))
					{
						http_response = get_v_servers()[j].treat_request(token, nbytes);
						std::cout << "in a virtual server" << std::endl;
						break;
					}
				}

				// need to check if the request method (GET, POST OR DELETE) is allowed on the location
				// of the requested page --> look in the locations directives
				if (http_response.empty())
					http_response = this->treat_request(token, nbytes);
				
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
		i++;
	}
	return (1);
}

std::pair<bool, Location> Server::match_location(std::string requested_page)
{
	for (unsigned int i = 0; i < get_rules().get_locations().size(); i++)
	{
		std::string location_url = this->get_rules().get_locations()[i].get_prefix();
		//looks for a location block for which the requested page url has a prefix that matches with the 
		//location prefix
		if (!requested_page.rfind(location_url, 0))
			return (std::make_pair(true, this->get_rules().get_locations()[i]));
	}
	return (std::make_pair(false, Location()));
}

std::string Server::treat_request(char *token[3], int nbytes)
{
	std::string full_path;
	std::string path;
	(void)nbytes;
	//Check client_max_body_size to see if the request is not too long
	
	// if (nbytes > rule_set.get_client_max_body_size())
	// return (std::string());
	display_IP();
	// see if the page requested matches a location in our rules
	std::pair<bool, Location> found = match_location(token[1]);

	std::cout << std::boolalpha << std::endl;
	std::cout << "Did we match the url searched with our prefix : " << found.first << std::endl;
	//page requested is a page included in a location block

	if (found.first)
	{
		//our set of rules from the location match
		Location location = found.second;

		//our file path inside our server's directories (file that we actually need to open)
		path = location.get_location_rules()["root"];
		//requested page url
		std::string requested_page = std::string(token[1]);
		

		//add to our path a substring of our requested page beginning from the point our 
		//location prefix ends. 
		// if url was /upload/lol/exercices/ and prefix of location was /upload/lol/ rooted to ./
		// then we need to look were the url continues, and add that to the back of our root

		path += requested_page.substr(location.get_prefix().length());
		if (requested_page.back() == '/')
			path += location.get_location_rules()["index"]; //change to variable in the ruleset of the server
	}
	//we are getting a GET request on server
	if (!strcmp(token[0], "GET"))
	{
	//treating HTTP/1.1 request
		if (!strcmp(token[2], "HTTP/1.1"))
		{
			//check if file exists, if it doesn't we need to send back the correct http response
			FILE *file_fd = fopen(path.c_str(), "r");
			std::string http_response;
			if (file_fd)
				http_response = get_response(path, token[2] , 200);
			else //404 page not found, fopen didn't find the page requested. Change the 404.hmtl by the correct default error page coming from the conf file
			{
				// std::cout << "Couldn't find file : " << path << std::endl;
				http_response = get_response(std::string(path) + "/404.html", token[2], 404);
			}
			fclose(file_fd);
			return (http_response);
		}
	}
	//remove, just present for testing
	return (std::string());
}
