/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zelhajou <zelhajou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/08 17:42:18 by zelhajou          #+#    #+#             */
/*   Updated: 2024/08/11 02:34:42 by beddinao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Sockets.hpp"
#include <unistd.h>
#include <pwd.h>

Sockets		S;

void	sig_nan(int sig_num)
{
	std::cout << KCYN << "main_process:" << KNRM
		<< " received signal(" << KGRN << sig_num
		<< KNRM << ") exiting..\n";
	exit(sig_num);
}

void	leaks_fun(void)
{
	system("leaks webserv");
}

std::string readFile(const std::string& filepath)
{
	std::ifstream file(filepath.c_str());
	if (!file.is_open()) {
		std::cout << "Unable to open file: " + filepath << std::endl;
		if (std::strcmp(filepath.c_str(), DEFAULT_CONFIG)) {
			std::cout << "falling to default: " << DEFAULT_CONFIG << std::endl;
			return	readFile(DEFAULT_CONFIG);
		}
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::string get_cwd(char *buf, size_t size)
{
	char *cwd = getcwd(buf, size);
	if (cwd == NULL)
		return "";
	std::string cwd_str(cwd);
	free(cwd);
	return cwd_str;
}

MainConfig push_valid_servers(MainConfig &main_config)
{
	MainConfig main_config_validated;
	for (size_t i = 0; i < main_config.servers.size(); ++i)
	{
		if (main_config.servers[i]->valid)
			main_config_validated.servers.push_back(main_config.servers[i]);
	}
	return main_config_validated;
}

int main(int argc, char *argv[], char **env)
{
	fix_up_signals(sig_nan);
	std::string	config_file;
	if (argc > 2) {
		std::cerr << KRED << "->\ttoo many arguments" << KNRM
			<< "\tUsage: " << argv[0] << " ?[config_file]" << std::endl;
		return 1;
	}
	config_file = (argc == 2 ? argv[1] : DEFAULT_CONFIG);
	try {
		std::string config_content = readFile(config_file);	
		Tokenizer	tokenizer(config_content);
		std::vector<Token>	tokens = tokenizer.tokenize();

		Parser	parser(tokens);
		MainConfig	main_config = parser.parse();

		std::string cwd = get_cwd(NULL, 0);
		ConfigValidator validator(main_config, cwd);
		validator.validate();
		MainConfig main_config_validated = push_valid_servers(main_config);

		S.initiate_servers(main_config_validated, env);
		S.run();
	}
	catch (std::exception &e) { std::cerr << KRED << e.what() << KNRM << std::endl; }
	return 0;
}
