#include "Server.hpp"
#include "webserv.hpp"

//TODO

std::pair<bool, std::string> Server::treat_get_request(Request &req, Location &location, std::string path, std::string server_directory)
{
	(void)location;
	(void)server_directory;

    //treating HTTP/1.1 request
    if (!req.protocol.compare("HTTP/1.1"))
    {
        if (file_is_php(req.uri)) {
            std::cout << "GET PHP REQUEST" << std::endl;
            return (php_cgi(req, server_directory, path, location));
        }
        else {
            std::string http_response = get_response(path, req.uri, req.protocol , 200, 0);
            return (std::make_pair(true,http_response));
        }
    }
	return (std::make_pair(false, std::string()));
}

