#include "Server.hpp"
#include "Request.hpp"
#include "errno.h"


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

void Server::push_fd(std::vector<struct pollfd> &all_pfds, int fd, int events)
{
	struct pollfd new_fd;

	new_fd.fd = fd;
	new_fd.events = events;
	fcntl(new_fd.fd, F_SETFL, O_NONBLOCK);
	pfds.push_back(new_fd);
	all_pfds.push_back(new_fd);
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

//send all the data
int Server::send_all(int fd, std::string http_response, int *len)
{
	int total = 0;
	int bytes_left = *len;
	int n;
	while (total < *len)
	{
		n = send(fd, http_response.c_str(), http_response.length(), 0);
		if (n == -1)
			break ;
		total += n;
		bytes_left -= n;
	}
	*len = total; //actual number sent
	return (n == -1 ? -1 : 0); //return -1 on failure of send, 0 otherwise
}

void	reformat_data(Request & request)
{
	std::istringstream  data_stream(request.data);
	int                 find_filename = 0;
	std::string         filename;
	std::string			line;

	while (std::getline(data_stream, line))
	{
		if (line == "\r")
			break;
		find_filename = line.find("filename=\"");
		if (find_filename >= 0) {
			filename = line.substr(find_filename + 10);
			filename.resize(filename.size() - 2);
			request.headers["Filename"] = filename;
			break;
		}
	}
}

std::pair<int, Request>	Server::receive_http_request(int i)
{
	char		buff[512];
	std::string	full_request;
	int			nbytes;
	int			find;
	Request		request;

	while (1)
	{
		poll(&(pfds[i]), 1, -1);
		nbytes = recv(pfds[i].fd, buff, sizeof(buff), 0);
		if (nbytes == 0)
			return (std::make_pair(nbytes, request));
		if (nbytes < 0)
			return (std::make_pair(nbytes, request));
		full_request += std::string(buff, nbytes);
		memset(buff, 0, sizeof(buff));
		find = full_request.find("\r\n\r\n");
		if (find >= 0) {
			request.fill_object(full_request);
			break;
		}
	}

	if (request.type == "POST")
	{
		long				to_read;
		std::stringstream	ss_content_length(request.headers["Content-Length"]);
		unsigned int		content_length = 0;

		ss_content_length >> content_length;
		to_read = content_length - request.data.size();

		while (to_read > 0)
		{
			poll(&(pfds[i]), 1, -1);
			nbytes = recv(pfds[i].fd, buff, sizeof(buff), 0);
			if (nbytes == 0)
				return (std::make_pair(nbytes, request));
			if (nbytes < 0)
				return (std::make_pair(nbytes, request));
			request.data += std::string(buff, nbytes);
			memset(buff, 0, sizeof(buff));
			to_read -= nbytes;
		}
		if (request.headers["Content-Type"] == "multipart/form-data;")
			reformat_data(request);
	}
	return (std::make_pair(nbytes, request));
}

int Server::poll_fds(std::vector<struct pollfd> &all_pfds, int all_index, int server_index)
{
	//buffer to hold the ip of our incoming connection
	char remote_ip[INET6_ADDRSTRLEN];

	//struct of an incoming connection
	struct sockaddr_storage remoteaddr;
	socklen_t remoteaddr_len;

	//fd of our new connection
	int new_connection;


	//go through our array to check if one fd is ready to read

	//our socket_fd is the one ready to read, this means a new incoming connection
	if (all_pfds[all_index].fd == sock.get_socket_fd())
	{
		remoteaddr_len = sizeof(remoteaddr);

		//accept the connection
		new_connection = accept(sock.get_socket_fd(), (struct sockaddr *)&remoteaddr, &remoteaddr_len);
		if (new_connection == -1)
			std::cerr << "error on accepting new connection" << std::endl;
		else
		{
			push_fd(all_pfds, new_connection, POLLIN);
			std::cout << "new connection from " << inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remote_ip, INET6_ADDRSTRLEN) << std::endl;
		}
	}
	else //means that this is not out socket_fd, so this is a normal connection being ready to be read, so an http request is there
	{
		std::pair<int, Request>	pair_bytes_request;
		pair_bytes_request = receive_http_request(server_index);

		if (pair_bytes_request.first <= 0)
		{
			if (pair_bytes_request.first == 0) //connection closed
				std::cout << "Connection closed by client at socket " << all_pfds[all_index].fd << std::endl;
			if (pair_bytes_request.first < 0)
				std::cerr << strerror(errno) << std::endl;

			close(all_pfds[all_index].fd);
			pfds.erase(pfds.begin() + server_index);
			all_pfds.erase(all_pfds.begin() + all_index);
			return (0) ;
		}

		Request	request = pair_bytes_request.second;

		if (request.type.empty() || request.uri.empty() || request.protocol.empty())
			return (0) ;				

		// We need to parse the request to get the hostname!!!
		std::string hostname = request.headers["Host"];
		std::pair<bool, std::string> request_treated;
		std::string http_response = "";

		for (unsigned int j = 0; j < this->get_v_servers().size(); j++)
		{
			//if we find a virtual server whose server_name directives matches with the Host field
			// of our request, that's the one that should treat it
			if (!get_v_servers()[j].get_rules().directives["server_name"].compare(hostname))
			{
				request_treated = get_v_servers()[j].treat_request(request);
				http_response = request_treated.second;
				break;
			}
		}

		// need to check if the request method (GET, POST OR DELETE) is allowed on the location
		// of the requested page --> look in the locations directives
		if (http_response.empty())
		{
			request_treated = this->treat_request(request);
			http_response = request_treated.second;
		}

		all_pfds[all_index].events = POLLOUT;
		poll(all_pfds.data() + all_index, 1, 0);

		if (all_pfds[all_index].revents & POLLOUT)
		{
			int len = http_response.length();
			int bytes_sent = send_all(all_pfds[all_index].fd, http_response, &len);
			//number of bytes send differs from the size of the string, that means we had a problem with send()
			if (bytes_sent == -1 || len != static_cast<int>(http_response.length()))
			{
				if (bytes_sent == -1)
					std::cerr << "error on send" << std::endl;
				else
					std::cerr << "send didn't write all the package" << std::endl;
				close_connection(all_pfds, server_index, all_index);
			}
			//not sure these are necessary if recv and send worked
			all_pfds[all_index].events = POLLIN;
			if (!request_treated.first)
				close_connection(all_pfds, server_index, all_index);
		}
	}
	return (1);
}

int	Server::close_connection(std::vector<struct pollfd> &all_pfds, int server_index, int all_index)
{
	shutdown(all_pfds[all_index].fd, SHUT_RDWR);
	close(all_pfds[all_index].fd);
	all_pfds.erase(all_pfds.begin() + all_index);
	pfds.erase(pfds.begin() + server_index);
	return (1);
}

std::pair<bool, Location> Server::match_location(std::string requested_page)
{

	for (unsigned int i = 0; i < get_rules().locations.size(); i++)
	{
		std::string location_url = this->get_rules().locations[i].prefix;
		//looks for a location block for which the requested page url has a prefix that matches with a location
		if (!requested_page.rfind(location_url, 0))
			return (std::make_pair(true, this->get_rules().locations[i]));
	}
	return (std::make_pair(false, Location()));
}

//false if response is a redirect 301, true otherwise
std::pair<bool, std::string> Server::treat_request(Request &req)
{
	std::string full_path;
	std::string path;
	Location location;
	std::string server_directory;

	// see if the page requested matches a location in our rules
	std::pair<bool, Location> found = match_location(req.uri);

	//page requested is a page included in a location block
	location = found.second;
	

	//our set of rules from the location match

	//our file path inside our server's directories (file that we actually need to open)
	path = location.location_map["root"];

	//add to our path a substring of our requested page beginning from the point our 
	//location prefix ends. 
	// if url was /upload/lol/exercices/ and prefix of location was /upload/lol/ rooted to ./
	// then we need to look were the url continues, and add that to the back of our root
	path += req.uri.substr(location.prefix.length());

	server_directory = path;

	server_directory = server_directory.substr(0, server_directory.rfind("/") + 1);

	if (location.location_map[req.type] != "true")
		return (std::make_pair(false, get_response(location.location_map["GET"], location.location_map["POST"], location.location_map["DELETE"], 405, 0)));

	if (location.location_map["return"].length() != 0)
	{
		std::string return_cpy = location.location_map["return"];
		int status = atoi(split(return_cpy, " ").c_str());
		std::string path = split(return_cpy, " ");
		if (status >= 400 && status < 500)
			return (std::make_pair(true, get_response(path, req.uri, req.protocol, status, 0)));
		else if (status >= 300 && status < 400)
			return (std::make_pair(false, get_response(path, path, req.protocol, status, 0)));
	}

	if (req.uri.back() == '/') //if it's a directory
	{
		path += location.location_map["index"];
		if (location.location_map["autoindex"] == "on" && !found_file(path))
			return (std::make_pair(true, get_response(server_directory, req.uri, req.protocol, 1, 0)));
	}
	else
	{
		struct stat s;
		if (!stat(path.c_str(), &s))
		{
			if (s.st_mode & S_IFDIR) //path is a directory but not ended by a '/'
			{
				return (std::make_pair(false, get_response(path, req.uri + "/", req.protocol, 301, 0)));
			}
		}
	}
	//if we got to here, we either have : 
	//	1. the path was a directory so we added the index directive to it
	//	2. the path is a file, we don't know if it's valid or not yet
	//check here for 404 file not found
	if (!found_file(path))
	{

		path = server_directory + location.location_map["error_page"];
		if (!found_file(path))
			return (std::make_pair(true, generate_error_page("Default error page", "404 Page not found")));
		else
		{
			return (std::make_pair(true, get_response(path, req.uri, req.protocol, 404, 0)));
		}
	}

	//we are getting a GET request on server
	if (req.type == "POST")
		return (this->treat_post_request(req, location, path, server_directory));

	if (req.type == "GET")
		return (treat_get_request(req, location, path, server_directory));

	if (req.type == "DELETE")
		return (std::make_pair(true, treat_delete_request(path)));
	//remove, just present for compiling
	return (std::make_pair(false, "Don't send giberrish to our server"));
}
