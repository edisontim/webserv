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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <ctime>
#include <sstream>

#include <stdio.h>
#include <string.h>

typedef std::basic_stringstream<char> stringstream;

#include "Internet_socket.hpp"

//_____________HTTP webserver logic:_____________
//
//1. parse configuration file if given, if not have a default path to look into. Base the parsing on the "server" part of Nginx's conf file
//2. from this configuration file: 
//		a) create a bound socket file descriptor on the correct ip and port
//		b) set the working directory to the correct one where files need to be fetched
//		c) CGI settings
//		d) 
//3. listen to incoming connections
//4. Wait
//5. When a connection is received, wait for the fd to be ready to read the request
//6. Wait
//7. When a fd is ready to read, parse the request
//8. See if the request is correctly formulated, if not send a 400 HTTP response
//9. Is it a GET, POST, DELETE request ? 
//		a) GET : If it is correctly formulated see what page was requested : is it an HTML ? or do we have to use Common Gateway Interface
//			If CGI is needed, then fork the process to execute CGI ?? 
//		b) POST :
//		c) DELETE :
//10. Send response with chunked encoding ? If it's HTTP 1.0 then not, otherwise we might need to
//11. Shutdown connection and close fd ??



void *get_in_addr(struct sockaddr *address)
{
	if (address->sa_family == AF_INET)
		return &(((struct sockaddr_in *)address)->sin_addr);
	else
		return &(((struct sockaddr_in6 *)address)->sin6_addr);
}

//return content of HTML_FILE as a std::string
std::string file_content(FILE *file_fd)
{
	int f_size;
	char *buff;
	int result;

	//look for the end-of-file character
	fseek(file_fd , 0 , SEEK_END);

	//once the cursor is set to the position of the EOF, ftell gives the value of the offset, so we know how many characters are in the file
	f_size = ftell(file_fd);
	
	//go back to the beginning of our file
	rewind(file_fd);
	buff = (char *)malloc(sizeof(*buff) * (f_size + 1));
	buff[sizeof(*buff) * f_size] = 0;
	if (!buff)
		return (std::string());
	result = fread(buff, 1, f_size, file_fd);
	if (result != f_size)
	{
		free(buff);
		return (std::string());
	}
	std::string ret = buff;
	free(buff);
	return (ret);
}

std::string response_header(FILE *file_fd, std::string full_path, std::string http_v, int status)
{
	std::string response = http_v;
	std::string response_buff;
	if (status == 200)
		response += " 200 OK\r\n";
		
	//get current time
	char buff[1000];
	time_t now = time(0);
	struct tm ltm = *gmtime(&now);
	strftime(buff, sizeof(buff), "Date: %a, %d %b %Y %T %Z\r\n", &ltm);
	response += buff;
	response += "Server: Timserver\n\r";

	//check last modified time of file
	bzero((void*)buff,sizeof(buff));
	struct stat a;
	stat(full_path.c_str(), &a);
	struct tm last_modified = *gmtime(&a.st_mtime);
	strftime(buff, sizeof(buff), "Last modified: %a, %d %b %Y %T %Z\r\n", &last_modified);
	
	
	//get content of HTML file
	response += buff;
	response_buff += "Content-type: text/html\r\n"; 
	response_buff += "Connection: Closed\r\n\r\n";
	response_buff += file_content(file_fd); //HTML file

	response += "Content-length: ";
	int total_length;
	std::string buff_string;
	buff_string = response_buff.length() + response.length();
	total_length = buff_string.length() + response_buff.length() + response.length();
	stringstream ss;
	ss << total_length;
	response += ss.str();
	response += "\r\n";
	response += response_buff;
	return (response);
}


int main()
{
	//vector that holds all our fds, either our bound socket or a remote connection
	std::vector<struct pollfd> pfds;
	
	//socket fd
	Internet_socket bound_sock;

	//pfd is either our bound socket or a remote connection, index 0 is our bound sockets
	struct pollfd new_pfd; 
	int i;
	int new_connection;
	
	//struct of an incoming connection
	struct sockaddr_storage remoteaddr;
	socklen_t remoteaddr_len;
	
	//buffer to hold the data we receive from an incoming connection
	char buff[1000];

	//buffer to hold the ip of our incoming connection
	char remote_ip[INET6_ADDRSTRLEN];

	//hold the strings from our request's parsing
	char *token[3];

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

	//push our socket fd to our vector
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

					//accept the connection
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
				else //means that this is not out socket_fd, so this is a normal connection being ready to be read, so an http request is there
				{
					int nbytes = recv(pfds[i].fd, buff, sizeof(buff), 0);
					

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
							//path of files directory
							const char path[] = "./website";
							
							std::string full_path = path;
							
							full_path += token[1];
							//check if file exists, if it doesn't we need to send back the correct http response
							FILE *file_fd = fopen(full_path.c_str(), "r");
							if (file_fd)
							{
								std::string http_response = response_header(file_fd, full_path, token[2] , 200);
								std::cout << http_response << std::endl;
								send(pfds[i].fd, http_response.c_str(), sizeof(http_response.c_str()), 0);
								// shutdown(pfds[i].fd, SHUT_RDWR);
								close(pfds[i].fd);
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