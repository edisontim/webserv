#ifndef SERVER_HPP
#define SERVER_HPP

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
#include <map>
#include "webserv.hpp"

class Internet_socket;
class Virtual_server;

#include "Internet_socket.hpp"
#include "Rules.hpp"

class Server
{
	typedef std::vector<struct pollfd>::size_type size_type;

	private:

		//out socket file descriptor class, holds addrinfo(IP), socket_fd and service (port)
		Internet_socket				sock;

		//Servers that have the same IP and port as the default server (first one in the conf file)
		//Other servers are just a different server_name and different set of rules. They get the 
		//request from the listening server if the Request's header's host field if the same as their
		//server_name. They then get routed the request as a string, treat it with their own rules and
		//return it as a string for the Server to send it back to the correct connection.
		std::vector<Virtual_server>	v_servers;
	
		//vector that holds all our fds, either our bound socket in index 0 or a remote connection
		std::vector<struct pollfd>	pfds;
		
	protected:

		//this server_name
		std::string					name;

		//rule sets, determines how the Server or virtual server should treat the request
		Rules 						rule_set;
	
	public:
		Server(){};
		Server(Internet_socket const &socket_fd, std::string &server_name);
		Server(std::string new_server_name, const char* hostname = NULL, const char* service = "80");
		~Server(){};


//		___________GETTER/SETTERS___________

		Internet_socket				&get_sock(void);
		std::vector<Virtual_server>	&get_v_servers(void);
		std::vector<struct pollfd>	&get_pfds(void);
		void						set_rules(Rules &rules);

//		___________UTILS___________

		//add a new connection to our vector of fds
		void		push_fd(struct pollfd new_fd);
		void		push_fd(int fd, int events);

		//remove a connection from our vector of fds
		void		pop_fd(void);
		
		size_t		vector_size();
		
		//poll our fds to see if a request came through from a connection and send the response if there was
		int			poll_fds(void);
		void		add_v_server(std::string virtual_server);

		//treat the request according to the set of rules of our servers
		std::string	treat_request(const char* requested_page, const char* http_v, int nbytes);
};


class Virtual_server : public Server
{
	//inherited from Server :
	// std::string		name;
	// Rules			rule_set;

	private:

	public:
		Virtual_server(){};
		Virtual_server(std::string const &serv_name, Rules const &serv_rules);
		~Virtual_server(){};
};

//fills a vector of Servers up with pointer to new Listening Servers
size_t conf_file(std::string path, std::vector<Server *> &servers);


#endif
