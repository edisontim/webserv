#include "Server.hpp"

Virtual_server::Virtual_server(std::string const &serv_name, Rules const &serv_rules)
{
	name = serv_name;
	rule_set = serv_rules;
}
