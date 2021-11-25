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

int main()
{
	char ip_string[255];
	struct hostent *host;

	//this gets the official name of the host machine running the program
	gethostname(ip_string, INET6_ADDRSTRLEN);
	std::cout << ip_string << std::endl;
	
	// sends back a pointer to hostent struct, by casting the h_addr (which is a define to the first element of the h_addr_list) to an in_addr* and dereferencing that we can get an ip addr in the form of a in_addr
	// this is a valid method but I think getaddrinfo with hints.ai_flags set to AI_PASSIVE to just use our own IP is easier
	host = gethostbyname(ip_string);

	std::cout << inet_ntoa(*((struct in_addr *)host->h_addr)) << std::endl;
}