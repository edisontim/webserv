#include <unistd.h>
#include "Server.hpp"
#include "webserv.hpp"

bool    file_is_php(std::string & uri)
{
    int last_dot_position;
    std::string extension;

    last_dot_position = uri.find_last_of(".");
    if (last_dot_position <= 0)
        return (false);

    extension = uri.substr(last_dot_position + 1);
    if (extension == "php")
        return (true);
    return (false);
}

std::pair<bool, std::string> Server::treat_post_request(Request & request, Location &location, std::string path, std::string server_directory)
{
    std::string http_response;

    if (file_is_php(request.uri)) {
        // generate html and response with php-cgi
        return (php_cgi(request, server_directory, path, location));
    }
    else {
        // we need to look for the page in the directory where we are rn
        return (treat_get_request(request, location, path, server_directory));
    }
}

