#include "webserv.hpp"
#include "Rules.hpp"
#include "Server.hpp"

//TODO ON PARSING
//================
//parse location block

void	display_map(std::map<std::string, std::string> map)
{
	std::map<std::string, std::string>::iterator iter = map.begin();
	std::cout << "Rules of server : " << std::endl;
	std::cout << "-----------------" << std::endl;
	unsigned int longest_key = 0;
	while (iter != map.end())
	{
		if (iter->first.length() > longest_key)
			longest_key = iter->first.length();
		iter++;
	}
	iter = map.begin();
	while (iter != map.end())
	{
		unsigned int a = iter->first.length();
		std::string b = " --> ";
		while (a < longest_key + 4)
		{
			b += " ";
			a++;
		}
		std::cout << iter->first << b << iter->second << std::endl;
		iter++;
	}
	std::cout << std::endl;
}

//get a string from open curly brace to closing
std::pair<std::string, int> get_location_block(std::string file, int i)
{
	int begin = i;
	while (file[i])
	{
		if (file[i] == '}')
			break ;
		i++;
	}
	//returns the location block string as well as the position of the end of it
	return (std::make_pair(file.substr(begin, i), i)); 
}

//server/location block string, index of end of directive
std::string new_word(std::string block, int i)
{
	unsigned int word_begin;
	while (block[i] && isspace(block[i]))
		i++;
	word_begin = i;
	while (block[i] && !isspace(block[i]) && block[i] != ';')
		i++;
	std::string word = block.substr(word_begin, i - word_begin);
	//if the word is only a { then that means the url for the location wasn't specified
	if (!word.compare("{"))
		return (std::string());
	return (block.substr(word_begin, i - word_begin));
}

//function has to return the number of characters we treated from the server block
int treat_location(Rules &new_rules, std::string server_block, int i)
{
	(void)new_rules;
	unsigned int j = 0;
	int k;
	int size;

	std::pair<std::string, int> a = get_location_block(server_block, i);
	std::string location_string = a.first;
	unsigned int block_end = a.second;

	//sets the url while constructing our empty map of location rules. We need to fill our map
	Location new_location(new_word(location_string, 0));
	while (location_string[j] != '{')
		j++;

	//if no url was specified, skip that block
	if (!new_location.get_url().compare(""))
		return (block_end);
	while (j < location_string.length())
	{
		//skip whitespaces until a word is found
		while (j < location_string.length() && isspace(location_string[j]))
			j++;
		//get starting position of word
		k = j;

		//get ending position of word
		while (j < location_string.length() && !isspace(location_string[j]))
			j++;

		size = j - k;

		//check if the word matches one of the directives that we allow 
		//if it does add it to our map of location rules
		std::string keyword = location_string.substr(k, size);

		//go through our map of location rules, if a key matches the word we found, assign a new value to it
		std::map<std::string, std::string>::iterator iter = new_location.get_location_rules().begin();
		while (iter != new_location.get_location_rules().end())
		{
			if (!keyword.compare(iter->first))
				iter->second = new_word(location_string, j);
			iter++;
		}
	}
	std::cout << new_location.get_url() << std::endl;
	display_map(new_location.get_location_rules());
	new_rules.get_locations().push_back(new_location);
	return (block_end);
}


Rules parse_server(std::vector<Server *> &servers, std::string server_block)
{
	unsigned int i = 0;
	unsigned int j;
	unsigned int size = 0;
	Rules new_rules;
	(void)servers;

	while (server_block[i])
	{
		//skip whitespaces until a word is found
		while (i < server_block.length() && isspace(server_block[i]))
			i++;
		//get starting position of word
		j = i;

		//get ending position of word
		while (i < server_block.length() && !isspace(server_block[i]))
			i++;

		size = i - j;
		
		//check if the word matches one of the directives that we allow 
		//if it does and it's not location, store the next word in the second item of the pair
		//from our directives map
		std::string directives_keyword = server_block.substr(j, size);
		if (!directives_keyword.compare("location"))
		{
			//get a new Location set of rules from of location block and push it in our vector of locations rules for this server block
			i = treat_location(new_rules, server_block, i);
			continue ;
		}
		//go through our map of general server rules, if a key matches the word we found, assign a new value to it
		std::map<std::string, std::string>::iterator iter = new_rules.get_directives().begin();
		while (iter != new_rules.get_directives().end())
		{
			if (!directives_keyword.compare(iter->first))
				iter->second = new_word(server_block, i);
			iter++;
		}
	}
	return (new_rules);
}

//return the index of the start of the server block (right after the opening curly brace, so we're sure it's there)
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
			while (isspace(file[i]) && i < file.length())
				i++;
			if (file[i] && file[i] == '{')
				return (++i);
		}
		i++;
	}
	return (0);
}

std::string get_server_block(std::string &file)
{
	int server_begin;
	int i;
	unsigned char brace_flag;

	brace_flag = 0;
	server_begin = server_keyword(file);
	if (!server_begin)
		return (std::string());
	i = server_begin;
	file = file.erase(0, server_begin);
	i = 0;
	while (file[i])
	{
		if (file[i] == '{')
			brace_flag = 1;
		if (file[i] == '}')
		{
			if (brace_flag)
				brace_flag = 0;
			else
				break;
		}
		i++;
	}
	return (file.substr(0, i));
}

bool	only_digits(std::string str)
{
	int i = 0;
	while (str[i])
	{
		if (!isdigit(str[i]))
			return (false);
		i++;
	}
	return (true);
}

std::pair<std::string, std::string> split_listen(Rules &rules)
{
	if (only_digits(rules.get_directives()["listen"]))
		return (std::make_pair("", rules.get_directives()["listen"]));
	else
	{
		//position of delimiter
		unsigned int pos = rules.get_directives()["listen"].find(":");
		return (std::make_pair(rules.get_directives()["listen"].substr(0, pos), rules.get_directives()["listen"].substr(pos + 1, rules.get_directives()["listen"].length())));
	}
}

//add a server to our vector of listening Servers, or a virtual server to a Listening server's vector
void add_server(std::vector<Server *> &servers, Rules &rules)
{
	unsigned int i = 0;

	//split the listen directive of our rule to get the port and IP separetely
	std::pair<std::string, std::string> IP_port = split_listen(rules);

	//if the IP is empty, this means that we'll listen to all the available interfaces on this port. 
	//An empty IP means listening to 0.0.0.0
	if (!IP_port.first.compare(""))
		rules.get_directives()["listen"] = "0.0.0.0" + IP_port.second;
	while (i < servers.size())
	{
		if (!servers[i]->get_rules().get_directives()["listen"].compare(rules.get_directives()["listen"]))
		{
			servers[i]->push_v_server(Virtual_server(rules));
			return ;
		}
		i++;
	}
	//get the IP and the PORT of the server
	//need to replace all correct values of the IP:port, and server_name here.
	if (!IP_port.first.compare(""))
		servers.push_back(new Server(IP_port.second.c_str(), "0.0.0.0"));
	else
		servers.push_back(new Server(IP_port.second.c_str(), IP_port.first.c_str()));
	servers[servers.size() - 1]->set_rules(rules);
}

//beginning of parsing

//fills a vector of Servers up with pointer to new Listening Servers
size_t conf_file(std::string path, std::vector<Server *> &servers)
{
	//our whole configuration file
	std::string file_string;

	Rules new_rules;

	//check if file exists, if it doesn't we can't launch the server and need to print the 
	//appropriate message;
	FILE *file_fd = fopen(path.c_str(), "r");
	if (!file_fd)
		return (0);
	fclose(file_fd);
	//get the content of the file as a string
	file_string = file_content(path);

	while (1)
	{
		//get the next "server" text block from file. Deletes the content in the string before it
		std::string server_string = get_server_block(file_string);

		//this means no more server blocks where found
		if (server_string.empty())
			return (1);
		
		//parse the "server" block and get the rules from it
		new_rules = parse_server(servers, server_string);
		add_server(servers, new_rules);
	}
	return (0);
}
