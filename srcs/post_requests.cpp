#include "Server.hpp"

std::string Server::treat_post_request(std::string buffer)
{
    (void)buffer;
    // std::cout << "-----BUFFER IS-----" << std::endl << buffer << std::endl;

    //  parse the post request, we need:
    //      - action attribute
    //      - HTTP/1.1
    //      - Host
    //      - Content-Length
    //      - Content-type
    //      - Data
    return ("ok");
}
