#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <map>
#include <sstream>
#include <iostream>
#include <cstdlib>

class Request
{
    private:

        Request();

    public:

        std::string                         type;
        std::string                         uri;
        std::string                         protocol;
        std::map<std::string, std::string>  headers;
        std::string                         data;

        Request(std::string request);
        ~Request();
        void    print();
};

#endif
