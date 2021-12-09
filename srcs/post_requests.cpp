#include "Server.hpp"

std::string Server::treat_post_request(Request & request)
{
    //  post request, we need:
    //      - uri
    //      - protocol
    //      - Host
    //      - Content-Length
    //      - Content-type
    //      - Data
    request.print();
    return ("200");
}
