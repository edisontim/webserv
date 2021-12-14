#include "webserv.hpp"
#include "Server.hpp"
#include <sstream>


template <typename T>
std::string itoa( T Number )
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}

std::string split(std::string &src, std::string delim)
{
	size_t pos = src.find(delim);
	std::string token = src.substr(0, pos);
	src.erase(0, pos + delim.length());
	return (token);
}

std::string generate_error_page(void)
{
	std::string body = 
"<!doctype html>\
<html>\
<head>\
<title>Nope</title>\
</head>\
<body>\
<p>Default error page</p>\
</body>\
</html>";
	std::string headers = 
"HTTP/1.1 404 Page not found\r\n\
Server: webserv\r\n\
Content-type: text/html\r\n\
Content-length: " + itoa(body.length());
headers += "\r\nConnection: Closed\r\n\r\n";
	return (headers + body);
}

std::string add_a_tag(std::string name)
{
	return ("<a href=\"" + name + "\">" + name + "</a>");
}

//path is the server's directory
std::string autoindex(std::string path, std::string uri)
{
	DIR *dir = opendir(path.c_str());
	if (!dir)
		return (std::string());
	struct dirent *current_entry;

	std::string body = 
"<!doctype html> \
<html>\
	<head>\
		<title>Nope</title>\
		<link rel=\"stylesheet\" href=\"mystyle.css\">\
		<link rel=\"icon\" type=\"image/png\" href=\"favicon.png\">\
	</head>\
<body>\
<h1>Index of " + uri + "</h1><hr><pre>";
	while ((current_entry = readdir(dir)))
	{
		body += add_a_tag(current_entry->d_name);
		body += "</br>";
	}
	body += 
"</pre><hr>\
</body>\
</html>";
	return (body);
}


void cleanup(int)
{
	exit(1);
}

int found_file(std::string path)
{
	FILE *file_fd = fopen(path.c_str(), "r");
	if (!file_fd)
		return (0);
	fclose(file_fd);
	return (1);
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
	ret.insert(std::make_pair("ico", "image/x-icon"));
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

std::string get_response(std::string path, std::string req_uri, std::string http_v, int status)
{
	//starts with HTTP/1.1 or HTTP/1.0
	std::string response;
	if (status == 405)
		response = "HTTP/1.1";
	else
		response = http_v;
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
		//path is actually the full url requested 
		response += "Location: " + req_uri + "\r\n\r\n";
		return (response);
	}

	if (status == 1)
		response += " 200 OK\r\n";
	
	if (status == 405)
	{
		response += " " + itoa(status) + " Method not allowed\r\n";
		response += "Content-type: text/html\r\n";
		response += "Allow: ";
		if (path == "true")
			response += "GET ";
		if (req_uri == "true")
			response += "POST ";
		if (http_v == "true")
			response += "DELETE";
		response += "\r\n\r\n";
		response += "<h1>405 Try another method!</h1>";
		return (response);
	}

	//get current time /!\\ careful, needs to be adapted to WINTER TIME
	response += "Date: ";
	response += dt_string(path, CURRENT);
	response += "\r\n";

	//name of the server and OS it's running on
	response += "Server: webserv\n\r";


	//get time of last modification of file
	response += "Last modified: ";
	response += dt_string(path, LAST_MODIFIED);
	response += "\r\n";

	//Content-type of file
	if (status == 1)
		response += "Content-type: text/html\r\n";
	else
	{
		std::map<std::string, std::string> file_types = file_extensions_map();
		extension = get_extension(file_types, path);
		response += "Content-type: ";
		response += extension;
		response += "\r\n";
	}

	//autoindex status
	if (status == 1)
	{
		body = autoindex(path, req_uri);
		response += "Content-length: " + itoa(body.length());
		response += "\r\n";
		response += "Connection: Closed\r\n\r\n";
		response += body;
		return (response);
	}
	else
	{
		//get content of HTML file
		// response += "Accept-Ranges: bytes\r\n";
		body = file_content(path);
	}
	if (body == "")
		return (std::string());


	response += "Content-length: ";
	unsigned int total_length;	

	stringstream ss;
	total_length = file_byte_dimension(path);
	ss << total_length;
	response += ss.str();
	response += "\r\n";

	//Connection type
	response += "Connection: Closed\r\n\r\n";
	response += body;
	return (response);
}

