/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zelhajou <zelhajou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/10 17:07:28 by zelhajou          #+#    #+#             */
/*   Updated: 2024/07/10 17:58:33 by zelhajou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

MainConfig ConfigParser::parse(const std::string& config_file)
{
	MainConfig		config;
	std::ifstream	file(config_file);
	
	if (!file.is_open())
	{
		std::cerr << "Error: could not open configuration file " << config_file << std::endl;
		exit(1);
		// return main_config;
	}
	std::string		line;
	while (std::getline(file, line))
	{
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue;
		if (line == "server {") {
			parseServerBlock(file, config);
		}
	}
	file.close();
	return config;
}

void ConfigParser::parseServerBlock(std::ifstream& file, MainConfig& main_config)
{
	ServerConfig	config;
	std::string		line;

	while (std::getline(file, line))
	{
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue;
		if (line == "}") {
			main_config.servers.push_back(config);
			break;
		}

}

std::string ConfigParser::trim(const std::string& str)
{
	std::string whitespaces = " \t\f\v\n\r";
	size_t first = str.find_first_not_of(whitespaces);	
	if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(whitespaces);
    return str.substr(first, last - first + 1);
}