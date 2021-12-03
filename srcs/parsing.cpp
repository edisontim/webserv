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
		std::cout << "Rules of server : " << iter->first << " | "<< iter->second << std::endl;
		iter++;
	}
}

std::vector<std::string> directives_vector(void)
{
	std::vector<std::string> ret;
	ret.push_back("listen");
	ret.push_back("server_name");
	ret.push_back("error_page");
	ret.push_back("client_max_body_size");
	return (ret);
}

void treat_location(Rules &new_rules, std::string server_block, int i)
{
	(void)new_rules;
	(void)server_block;
	(void)i;
}

//server block string, index of end of directive, index of the correct 
std::string new_word(std::string server_block, int i)
{
	unsigned int word_begin;
	while (server_block[i] && isspace(server_block[i]))
		i++;
	word_begin = i;
	while (server_block[i] && !isspace(server_block[i]) && server_block[i] != ';')
		i++;
	return (server_block.substr(word_begin, i - word_begin));
}

Rules parse_server(std::vector<Server *> &servers, std::string server_block)
{
	(void)servers;
	unsigned int i = 0;
	unsigned int j;
	unsigned int size = 0;
	unsigned int k;
	std::vector<std::string> directives = directives_vector();
	Rules new_rules;

	while (server_block[i])
	{
		k = 0;
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
			treat_location(new_rules, server_block, j);
			continue;
		}
		while (k < directives.size())
		{
			if (!directives_keyword.compare(directives[k]))
				new_rules.get_directives()[directives_keyword] = new_word(server_block, i);
			k++;
		}
		if (i < server_block.length())
			i++;
	}
	display_map(new_rules.get_directives());
	return (new_rules);
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
	//if the IP is empty, this means that we'll listen to the available 
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
		servers.push_back(new Server(IP_port.second.c_str()));
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
