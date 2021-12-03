#include "webserv.hpp"
#include "Rules.hpp"
#include "Server.hpp"

//TODO ON PARSING
//================
//managed to get the beginning of the server block
//need to parse that block now

void	display_map(std::map<std::string, std::string> map)
{
	std::map<std::string, std::string>::iterator iter = map.begin();
	while (iter != map.end())
	{
		std::cout << "Rules of server : " << iter->second << std::endl;
		iter++;
	}
}

std::vector<std::string> directives_vector(void)
{
	std::vector<std::string> ret;
	ret.push_back("listen");
	ret.push_back("server_name");
	ret.push_back("location");
	ret.push_back("error_page");
	ret.push_back("client_max_body_size");
	return (ret);
}

//server block string, index of end of directive, index of the correct 
std::string new_word(std::string server_block, int i)
{
	unsigned int word_begin;
	while (server_block[i] && isspace(server_block[i]))
		i++;
	word_begin = i;
	while (server_block[i] && !isspace(server_block[i]))
		i++;
	std::cout << server_block.substr(word_begin, i - word_begin) << std::endl;
	return (server_block.substr(word_begin, i - word_begin));
}

int parse_server(std::vector<Server *> &servers, std::string server_block)
{
	(void)servers;
	unsigned int i = 0;
	unsigned int j;
	unsigned int size = 0;
	unsigned int k = 0;
	std::vector<std::string> directives = directives_vector();
	Rules new_rules;


	while (server_block[i])
	{
		//skip whitespaces until a word is found
		while (isspace(server_block[i]) && server_block[i])
			i++;

		//get starting position of word
		j = i;

		//get ending position of word
		while (!isspace(server_block[i]) && server_block[i])
			i++;
		
		size = i - j;
		
		//check if the word matches one of the directives that we allow 
		//if it does and it's not location, store the next word in the second item of the pair
		//from our directives map
		std::string directives_keyword = server_block.substr(j, size);
		while (k < directives.size())
		{
			if (!directives_keyword.compare(directives[k]))
				new_rules.get_directives()[directives_keyword] = new_word(server_block, i);
			k++;
		}
	}
	return (0);
}

int server_keyword(std::string const &file)
{
	unsigned int i = 0;
	unsigned int j;
	unsigned int size = 0;
	std::string server_string;

	//skip whitespaces at the beginning
	while (file[i])
	{
		//skip whitespaces until a word is found
		while (isspace(file[i]) && file[i])
			i++;

		//get starting position of word
		j = i;

		//get ending position of word
		while (!isspace(file[i]) && file[i])
			i++;
		
		size = i - j;
		//get the substring from the begining of the word to the end and compare it to "server"
		//if the first word is server, this means we might have a server block
		if (!file.substr(j, size).compare("server"))
		{
		std::cout << "Here we are : " << file.substr(j, size) << std::endl;
			while (isspace(file[i]) && i < file.length())
				i++;
			if (file[i] && file[i] == '{')
				return (i);
		}
		i++;
	}
	return (0);
}

std::string get_server_block(std::string &file)
{
	int server_begin;
	int i;


	server_begin = server_keyword(file);
	if (!server_begin)
		return (std::string());
	i = server_begin;
	file = file.erase(0, i);
	while (!isspace(file[i]))
		i++;
	return (file.substr(server_begin, i - server_begin));
}

//beginning of parsing

//fills a vector of Servers up with pointer to new Listening Servers
size_t conf_file(std::string path, std::vector<Server *> &servers)
{
	std::string file_string;

	//check if file exists, if it doesn't we can't launch the server and need to print the 
	//appropriate message;
	FILE *file_fd = fopen(path.c_str(), "r");
	if (!file_fd)
		return (0);
	fclose(file_fd);
	//get the content of the file as a string
	file_string = file_content(path);
	// while (1)
	// {
		std::string server_string = get_server_block(file_string);
		std::cout << server_string << std::endl;
		if (server_string.empty())
			return (1);
		parse_server(servers, server_string);
	// }
	return (0);
}
