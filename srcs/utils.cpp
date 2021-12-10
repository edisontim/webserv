#include "webserv.hpp"
#include "Server.hpp"
#include <sstream>

//function launched before leaving the program 


template <typename T>
std::string itoa( T Number )
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}

void cleanup(int)
{
	exit(1);
}

void *get_in_addr(struct sockaddr *address)
{
	if (address->sa_family == AF_INET)
		return &(((struct sockaddr_in *)address)->sin_addr);
	else
		return &(((struct sockaddr_in6 *)address)->sin6_addr);
}

//associates a file extension with the proper content-type of an HTML response
std::map<std::string, std::string> file_extensions_map(void)
{
	std::map<std::string, std::string> ret;
	ret.insert(std::make_pair("html", "text/html"));
	ret.insert(std::make_pair("png", "image/png"));
	ret.insert(std::make_pair("jpeg", "image/jpeg"));
	ret.insert(std::make_pair("css", "text/css"));
	return (ret);
}

//gets file extension from a file path
std::string get_extension(std::map<std::string, std::string> file_extensions, std::string full_path)
{
	std::string a = file_extensions.find(full_path.substr(full_path.find_last_of(".") + 1))->second;
	return (a);
}

unsigned long file_byte_dimension(std::string full_path)
{
	struct stat stat_buf;
	int rc = stat(full_path.c_str(), &stat_buf);
	//stat : on success, 0 is returned
	return (rc == 0 ? stat_buf.st_size : 0);
}

//return content of HTML_FILE as a std::string
std::string file_content(std::string full_path)
{
	//create an input file stream to read our file and put it in our buffer
	std::ifstream content(full_path);
	stringstream buffer;
	buffer << content.rdbuf();
	//return the buffer to string to have all our file in a string. 
	//Careful if it's a binary file ! There might be some \0 inside it 
	//so when working with char * this might mean there will be undefined behaviour, 
	//for example ft_strlen will stop at the first \0 encountered
	return (buffer.str());
}

std::string dt_string(std::string full_path, DT which)
{
	char buff[1000];
	//we need the current time string
	if (which == CURRENT)
	{
		time_t now = time(0);
		struct tm ltm = *gmtime(&now);
		strftime(buff, sizeof(buff), "%a, %d %b %Y %T %Z", &ltm);
	}
	else //we need the last modified time string
	{
		struct stat a;
		if (stat(full_path.c_str(), &a) != 0)
			return (std::string());
		struct tm last_modified = *gmtime(&a.st_mtime);
		strftime(buff, sizeof(buff), "%a, %d %b %Y %T %Z", &last_modified);
	}
	return (buff);
}

std::string get_response(std::string full_path, std::string http_v, int status)
{
	std::string response = http_v;
	std::string body;
	std::string extension;

	// response status line
	if (status == 200)
		response += " " + itoa(status) + " OK\r\n";
	
	if (status == 404)
		response += " " + itoa(status) + " Page not found\r\n";
	
	if (status == 301)
	{
		response += " " + itoa(status) + " Moved permanently\r\n";
		//full_path is actually the full url requested 
		response += "Location: " + full_path + "\r\n\r\n";
		std::cout << response << std::endl;
		return (response);
	}

	//get current time /!\\ careful, needs to be adapted to WINTER TIME
	response += "Date: ";
	response += dt_string(full_path, CURRENT);
	response += "\r\n";

	//name of the server and OS it's running on
	response += "Server: webserv\n\r";


	//get time of last modification of file
	response += "Last modified: ";
	response += dt_string(full_path, LAST_MODIFIED);
	response += "\r\n";

	//Content-type of file
	std::map<std::string, std::string> file_types = file_extensions_map();
	extension = get_extension(file_types, full_path);
	response += "Content-type: ";
	response += extension;
	response += "\r\n"; 
	
	//get content of HTML file
	// response += "Accept-Ranges: bytes\r\n";
	body = file_content(full_path);
	if (body == "")
		return (std::string());


	response += "Content-length: ";
	unsigned int total_length;	

	stringstream ss;
	total_length = file_byte_dimension(full_path);
	ss << total_length;
	response += ss.str();
	response += "\r\n";

	//Connection type
	response += "Connection: Closed\r\n\r\n";
	response += body;
	std::cout << response << std::endl;

	return (response);
}

