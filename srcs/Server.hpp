#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.hpp"

#include "Internet_socket.hpp"

class Internet_socket;

class Server
{
//When parsing, if a new server block has the same ip and port as an existing one 
//(in our vector of servers), then create the Internet_socket by copy of the existing one.

	typedef std::vector<struct pollfd>::size_type size_type;

	private:

		//out socket file descriptor class, holds addrinfo(IP), socket_fd and service (port)
		Internet_socket				sock;

		//server name from configuration file
		std::string					server_name;
	
		//vector that holds all our fds, either our bound socket in index 0 or a remote connection
		std::vector<struct pollfd>	pfds;
		
	public:
		Server(){};
		Server(Internet_socket const &socket_fd, std::string server_name);
		Server(std::string new_server_name, const char* hostname = NULL, const char* service = "80");
		~Server(){};


//		___________GETTER/SETTERS___________

		Internet_socket				&get_sock(void);
		std::string					&get_server_name(void);
		std::vector<struct pollfd>	&get_pfds(void);

//		___________UTILS___________

		void		push_fd(struct pollfd new_fd);
		void		push_fd(int fd, int events);
		void		pop_fd(void);
		size_t		vector_size();
		int			poll_fds(void);
};


#endif
