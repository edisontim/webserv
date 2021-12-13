#include "Server.hpp"
#include <unistd.h>
#include <sys/stat.h>

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

// void    php_cgi()
// {

// }

std::string Server::treat_post_request(Request & request, Location &location, std::string path, std::string server_directory)
{
    std::string http_response;

    std::cout << "path: " << path << std::endl;
    std::cout << "request.uri: " << request.uri << std::endl;
    std::cout << "server_directory: " << server_directory << std::endl;

    if (file_is_php(request.uri)) {
        // php_cgi()
        std::cout << "calling cgi" << std::endl;
    }
    else {
        // we need to look for the page in the directory where we are rn
        return (treat_get_request(request, location, path, server_directory).second);
    }
    return (http_response);
}

