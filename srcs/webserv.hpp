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
#include <dirent.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "Request.hpp"
#include "Location.hpp"


#define RED "\033[0;31m"
#define NORMAL "\x1b[0m"

class Server;

typedef std::basic_stringstream<char> stringstream;

enum DT {CURRENT, LAST_MODIFIED};


void								*get_in_addr(struct sockaddr *address);
std::map<std::string, std::string>	file_extensions_map(void);
std::string							get_extension(std::map<std::string, std::string> file_extensions, std::string full_path);
unsigned long						file_byte_dimension(std::string full_path);
std::string							file_content(std::string full_path, int from_php);
std::string							dt_string(std::string full_path, DT which);
std::string							get_response(std::string full_path, std::string req_uri, std::string http_v, int status, int from_php);
void								cleanup(int);
int									found_file(std::string path);
std::string							generate_error_page(std::string p_text, std::string code_response);
std::string							split(std::string &src, std::string delim);

// cgi
bool                            file_is_php(std::string & uri);
std::pair<bool, std::string>    internal_server_error();
int                             fork_cgi_process(int tubes[2], int file_fd, char *cgi_args[3], char *env[13]);
void                            php_fill_env(Request & request, std::string path, char **env[10]);
std::pair<bool, std::string>    php_cgi(Request & request, std::string server_directory, std::string path, Location & location);
std::pair<bool, std::string>    internal_server_error();

#endif
