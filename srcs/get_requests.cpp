#include "Server.hpp"

std::pair<bool, std::string> Server::treat_get_request(Request &req, Location &location, std::string path, std::string server_directory)
{
    //treating HTTP/1.1 request
    if (!req.protocol.compare("HTTP/1.1"))
    {
        //check if file exists, if it doesn't we need to send back the correct http response
        FILE *file_fd = fopen(path.c_str(), "r");
        std::string http_response;
        if (file_fd)
            http_response = get_response(path, req.uri, req.protocol , 200);
        else //404 page not found, fopen didn't find the page requested. Change the 404.hmtl by the correct default error page coming from the conf file
        {
            // std::cout << "Couldn't find file : " << path << std::endl;
            http_response = get_response(server_directory + location.location_map["error_page"], req.uri, req.protocol, 404);
        }
        fclose(file_fd);
        return (std::make_pair(true,http_response));
    }
	return (std::make_pair(false, std::string()));
}
