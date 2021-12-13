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

bool    file_exists(std::string & path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}


std::string Server::treat_post_request(Request & request, Location &location, std::string path, std::string server_directory)
{
    std::string http_response;

    // path: ./website/postform.html
    // error_page: 404.html
    std::cout << "path: " << path << std::endl;

    // check if page requested (path variable) exists (if not, serve error page)
    if (!file_exists(path))
    {
        std::cout << "Couldn't find file : " << path << std::endl;
        http_response = get_response(server_directory + location.location_map["error_page"], request.uri, request.protocol, 404);
        std::cout << "response: " << http_response << std::endl;
    }
    else
    {
        if (file_is_php(request.uri))
        {

        }
        else
        {

        }
    }
    return (http_response);
}

