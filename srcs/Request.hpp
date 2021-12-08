#include <map>
#include <sstream>
#include <iostream>

#define GET     0
#define POST    1
#define DELETE  2


class Request
{
    private:

        Request();

    public:

        int                                 type;
        std::string                         uri;
        std::string                         protocol;
        std::map<std::string, std::string>  headers;
        std::string                         data;

        Request(char    *buffer);
        ~Request();
};
