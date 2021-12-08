#include "Request.hpp"

Request::Request(char   *buffer)
{
    std::string request = std::string(buffer);
    std::istringstream  f(request);
    std::string         line;

    std::getline(f, line);
    line.spl
    while (std::getline(f, line))
    {
        
    }
}

Request::~Request() {}
