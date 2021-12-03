#include "Internet_socket.hpp"
#include "Server.hpp"

#include "webserv.hpp"

typedef std::basic_stringstream<char> stringstream;
enum DT {CURRENT, LAST_MODIFIED};
class Internet_socket;
class Server;


int	ft_strlen(const char *str)
{
	int i;
	i = 0;
	while (str[i])
		i++;
	return (i);
}


void *get_in_addr(struct sockaddr *address)
{
	if (address->sa_family == AF_INET)
		return &(((struct sockaddr_in *)address)->sin_addr);
	else
		return &(((struct sockaddr_in6 *)address)->sin6_addr);
}

//associates a file extension with the proper content-type of an HTML response
std::map<std::string, std::string> file_extensions_map(void)
{
	std::map<std::string, std::string> ret;
	ret.insert(std::make_pair("html", "text/html"));
	ret.insert(std::make_pair("png", "image/png"));
	ret.insert(std::make_pair("jpeg", "image/jpeg"));
	ret.insert(std::make_pair("css", "text/css"));
	return (ret);
}

//gets file extension from a file path
std::string get_extension(std::map<std::string, std::string> file_extensions, std::string full_path)
{
	std::string a = file_extensions.find(full_path.substr(full_path.find_last_of(".") + 1))->second;
	return (a);
}

unsigned long file_byte_dimension(std::string full_path)
{
	struct stat stat_buf;
	int rc = stat(full_path.c_str(), &stat_buf);
	//stat : on success, 0 is returned
	return (rc == 0 ? stat_buf.st_size : 0);
}

//return content of HTML_FILE as a std::string
std::string file_content(std::string full_path)
{
	//create an input file stream to read our file and put it in our buffer
	std::ifstream content(full_path);
	stringstream buffer;
	buffer << content.rdbuf();
	//return the buffer to string to have all our file in a string. 
	//Careful if it's a binary file ! There might be some \0 inside it 
	//so when working with char * this might mean there will be undefined behaviour, 
	//for example ft_strlen will stop at the first \0 encountered
	return (buffer.str());
}

std::string dt_string(std::string full_path, DT which)
{
	char buff[1000];
	//we need the current time string
	if (which == CURRENT)
	{
		time_t now = time(0);
		struct tm ltm = *gmtime(&now);
		strftime(buff, sizeof(buff), "%a, %d %b %Y %T %Z", &ltm);
	}
	else //we need the last modified time string
	{
		struct stat a;
		if (stat(full_path.c_str(), &a) != 0)
			return (std::string());
		struct tm last_modified = *gmtime(&a.st_mtime);
		strftime(buff, sizeof(buff), "%a, %d %b %Y %T %Z", &last_modified);
	}
	return (buff);
}

std::string response(std::string full_path, std::string http_v, int status)
{
	std::string response = http_v;
	std::string body;
	std::string extension;

	// response status line
	if (status == 200)
		response += " 200 OK\r\n";
	
	if (status == 404)
		response += " 404 Page not found\r\n";

	//get current time /!\\ careful, needs to be adapted to WINTER TIME
	response += "Date: ";
	response += dt_string(full_path, CURRENT);
	response += "\r\n";

	//name of the server and OS it's running on
	response += "Server: webserv\n\r";


	//get time of last modification of file
	response += "Last modified: ";
	response += dt_string(full_path, LAST_MODIFIED);
	response += "\r\n";

	//Content-type of file
	std::map<std::string, std::string> file_types = file_extensions_map();
	extension = get_extension(file_types, full_path);
	response += "Content-type: ";
	response += extension;
	response += "\r\n"; 
	
	//get content of HTML file
	body = file_content(full_path);
	if (body == "")
		return (std::string());


	response += "Content-length: ";
	unsigned int total_length;	

	stringstream ss;
	total_length = file_byte_dimension(full_path);
	ss << total_length;
	response += ss.str();
	response += "\r\n";

	//Connection type
	response += "Connection: Closed\r\n\r\n";
	response += body;
	return (response);
}



int main()
{
	//vector that holds all our fds, either our bound socket or a remote connection
	std::vector<struct pollfd> pfds;

	//socket fd
	Internet_socket bound_sock;

	if (bound_sock.get_socket_fd() < 0)
	{
		std::cerr << "Error while creating socket fd" << std::endl;
		return (1);
	}

	//pfd is either our bound socket or a remote connection, index 0 is our bound sockets
	struct pollfd new_pfd; 
	unsigned int i;
	int new_connection;
	
	//struct of an incoming connection
	struct sockaddr_storage remoteaddr;
	socklen_t remoteaddr_len;
	
	//buffer to hold the data we receive from an incoming connection
	char buff[9999];

	//buffer to hold the ip of our incoming connection
	char remote_ip[INET6_ADDRSTRLEN];

	//hold the strings from our request's parsing
	char *token[3];

	//get and print owns IPv4 address
	{
		char ip_string[255];
		struct hostent *host;

		//this gets the official name of the host machine running the program
		gethostname(ip_string, INET6_ADDRSTRLEN);

		// sends back a pointer to hostent struct, by casting the h_addr (which is a define to the first element of the h_addr_list) to an in_addr* and dereferencing that we can get an ip addr in the form of an in_addr
		// this is a valid method but I think getaddrinfo with hints.ai_flags set to AI_PASSIVE to just use our own IP is easier
		host = gethostbyname(ip_string);
		std::cout << "Users can connect to <" << inet_ntoa(*((struct in_addr *)host->h_addr)) << "> on port <" << bound_sock.get_service() << ">"<< std::endl;
	}

	//push our socket fd to our vector
	new_pfd.fd = bound_sock.get_socket_fd();
	new_pfd.events = POLLIN;
	pfds.push_back(new_pfd);

	//check until program is quit for an fd in our vector that has been set to POLLIN (ready to read)
	while (1)
	{
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
				if (pfds[i].fd == bound_sock.get_socket_fd())
				{
					remoteaddr_len = sizeof(remoteaddr);

					//accept the connection
					new_connection = accept(bound_sock.get_socket_fd(), (struct sockaddr *)&remoteaddr, &remoteaddr_len);
					if (new_connection == -1)
						std::cerr << "error on accepting new connection" << std::endl;
					else
					{
						new_pfd.fd = new_connection;
						new_pfd.events = POLLIN | POLLOUT;
						pfds.push_back(new_pfd);
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
	}
	return (0);
}
