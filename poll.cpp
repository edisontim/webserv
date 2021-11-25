#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
	struct pollfd pfds[1];
	char stdin_buff[255];
	pfds[0].fd = 0;
	pfds[0].events = POLLIN;
	std::cout << "Hit RETURN to have input read or wait 5 seconds for TIMEOUT" << std::endl;
	int num_event = poll(pfds, 1, 5000);
	if (num_event == 0)
	{
		std::cout << "TIMEOUT occured" << std::endl;
	}
	else
	{
		int pollin_happened = pfds[0].revents & POLLIN;
		if (pollin_happened)
		{
			std::cout << "File descriptor " << pfds[0].fd << " is ready to read" << std::endl;
			read(pfds[0].fd, stdin_buff, 255);
			std::cout << stdin_buff << std::endl;
		}
		else
		{
			std::cout << "Unexpected error occured" << std::endl;
		}
	}
	return (0);
}