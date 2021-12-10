#include "Request.hpp"

Request::Request(std::string request)
{
    std::istringstream  iss(request);
    std::string         line;
    std::istringstream  line_stream;
    std::string         header_key;
    std::string         header_value;

    std::getline(iss, line);
    line_stream.str(line);
    line_stream >> this->type >> this->uri >> this->protocol;

    while (std::getline(iss, line))
    {
        if (line == "\r")
            break;
        line_stream.str(line);
        line_stream >> header_key >> header_value;
        header_key.resize(header_key.size() - 1);
        this->headers[header_key] = header_value;
    }
    if (this->type == "POST")
    {
        std::getline(iss, line);
        this->data = line;
    }
}

Request::~Request() {}

void    Request::print()
{
    std::map<std::string, std::string>::iterator    it = this->headers.begin();
    std::map<std::string, std::string>::iterator    ite = this->headers.end();

    std::cout << "-----PRINT REQUEST-----" << std::endl;
    std::cout << this->type << " " << this->uri << " " << this->protocol << std::endl;
    while (it != ite)
    {
        std::cout << it->first << ": " << it->second << std::endl;
        ++it;
    }
    if (this->type == "POST")
        std::cout << std::endl << "DATA: " << this->data << std::endl;
}