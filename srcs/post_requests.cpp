#include "Server.hpp"

std::string Server::treat_post_request(Request & request, std::string path, std::string error_page)
{
    (void)path;
    (void)error_page;
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
