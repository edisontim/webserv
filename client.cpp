#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];
	if (argc < 3)
	{
		std::cout << "Usage: hostname port" << std::endl;
		return (1);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		std::cerr << "Error, no such host" << std::endl;
		return (1);
	}
	server = gethostbyname(argv[1]);
	if (server == NULL)
	{
		std::cerr << "Error no such host" << std::endl;
		return (1);
	}
	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	// std::cout << server->h_addr << std::endl;
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
	{
		std::cerr << "Error connecting" << std::endl;
		return (1);
	}
	std::cout << "Please enter the message to communicate" << std::endl;
	bzero(buffer, 256);
	fgets(buffer, 255, stdin);
	n  = write(sockfd, buffer, 255);
	if (n < 0)
		std::cerr << "Error writing to socket" << std::endl;
	n  = read(sockfd, buffer, 255);
	if (n < 0)
		std::cerr << "Error reading from socket" << std::endl;
	std::cout << buffer << std::endl;
	close(sockfd);
	return (0);
}