#ifndef WEBSERV_HPP
#define WEBSERV_HPP

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

typedef std::basic_stringstream<char> stringstream;
enum DT {CURRENT, LAST_MODIFIED};


void *get_in_addr(struct sockaddr *address);
std::map<std::string, std::string> file_extensions_map(void);
std::string get_extension(std::map<std::string, std::string> file_extensions, std::string full_path);
unsigned long file_byte_dimension(std::string full_path);
std::string file_content(std::string full_path);
std::string dt_string(std::string full_path, DT which);
std::string response(std::string full_path, std::string http_v, int status);


#endif