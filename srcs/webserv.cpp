#include "webserv.hpp"
#include "Server.hpp"
#include "Rules.hpp"

// A SERVER HAS :
// 1. an ip:port
//		Do we have to do IPv4 and IPv6 ?
// 2. a server_name
// 3. directives on how to handle files, requests, error pages, etc.
// 4. established connections
// 5. A listening socket
// Each one has to work in a different manner, but they can have the same ip and port. 
// Just not the same server_name, if they do, only the first one will be taken into account.
// SO they can share the same socket file descriptor.



//_____________HTTP webserver logic:_____________
//
//1. parse configuration file if given, if not, have a default path to look into. Base the parsing on the "server" part of Nginx's conf file
//		see http://nginx.org/en/docs/http/ngx_http_core_module.html for documentation and response statuses on nginx directives
//		- Chose the host:port of each "server"												listen
// 		- Setup of the server_name															server_name
// 		- The first server for a host:port will be the default server for this host:port.
// 		(it will answer all the requests that don't belong to another server)
// 		- Limiting the size of the body for the clients										client_max_body_size
//		- Setup of default error pages														error_page
//		- Setup of the routes with one or multiple rules (routes won't be using regexes)	location
//				--> see pdf
//		- CGI execution for certain file extensions
//2. from this configuration file: 
//		a) For each specified port and IP, create a bound socket file descriptor on the correct ip and port.
//		b) For each server name on the same port and IP, have a different array of pfds that store the incoming connections on these "servers"
//		c) set the working directory to the correct one where files need to be fetched
//		d) CGI settings
//		e) 
//3. listen for incoming connections
//4. Wait
//5. When a connection is received, wait for the fd to be ready to read the request
//6. Wait
//7. When a fd is ready to read, parse the request
//8. Check the request's header Host field and decide to which server_name the request should be routed to. 
//	 If its value does not match any server name, or the request doesn't contain this field at all,
//	 then route the request to the default server on this port.
//9. See if the request is correctly formulated, if not send a 400 HTTP response
//10. Is it a GET, POST, DELETE request ? 
//		a) GET : If it is correctly formulated see what page was requested : is it an HTML ? or do we have to use Common Gateway Interface
//			If CGI is needed, then fork the process to execute CGI ?? 
//		b) POST :
//		c) DELETE :
//11. Check to see if fd is ready to write. (POLLOUT) Send response with chunked encoding ? If it's HTTP 1.0 then not, otherwise we might need to
//12. Shutdown connection and close fd ??
//

//TODO
// careful for the correction pdf, we need to poll for both read and write at the same time !!
// WHY AM I CREATING TWO SOCKETS ?
// set the sockets to non-blocking
// Catch the CTRL-C (SIGINT), 

int main(int argc, char *argv[])
{
	unsigned int i;

	//input buffer
	std::string buff;

	//vector to hold pointers to each of our listening servers
	std::vector<Server *> servers;

	//avoid quitting program without being able to cleanup the vector and disconnecting the sockets. Really necessary ?
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGINT, &cleanup);

	//if we have a path to a configuration file then parse it, otherwise parse the default 
	//configuration file in conf.d/. If the argument provided wasn't an openable file 
	// or, if not provided, the default config file wasn't openable, return an error
	

	if (argc == 2)
	{
		if (!conf_file(argv[1], servers))
		{
			std::cerr << "Config file not found" << std::endl;
			return (1);
		}
	}
	else
	{
		if (!conf_file("conf.d/webserv.conf", servers))
		{
			std::cerr << "Default config file not found and none was provided" << std::endl;
			return (1);
		}
	}

	i = 0;
	while (i < servers.size())
	{
	//get and print own IPv4 address
		char ip_string[255];
		struct hostent *host;

		//this gets the official name of the host machine running the program
		gethostname(ip_string, INET6_ADDRSTRLEN);

		// sends back a pointer to hostent struct, by casting the h_addr (which is a define to the first element of the h_addr_list) to an in_addr* and dereferencing that we can get an ip addr in the form of an in_addr
		// this is a valid method but I think getaddrinfo with hints.ai_flags set to AI_PASSIVE to just use our own IP is easier
		host = gethostbyname(ip_string);
		std::cout << "Users can connect to <" << inet_ntoa(*((struct in_addr *)host->h_addr)) << "> on port <" << servers[i]->get_sock().get_service() << ">"<< std::endl;
		std::cout << "socket fd : " << servers[i]->get_pfds()[0].fd << std::endl;
		i++;
	}
	while (1)
	{
		i = 0;
		while (i < servers.size())
		{
			servers[i]->poll_fds();
			i++;
		}
	}

	// std::cout << "Socket fd : " << test.get_sock().get_socket_fd() << std::endl;
	// while (1)
	// {
	// 	test.poll_fds();
	// }
}
