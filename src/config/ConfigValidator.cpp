/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zelhajou <zelhajou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/06 13:39:35 by zelhajou          #+#    #+#             */
/*   Updated: 2024/08/14 12:18:53 by zelhajou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigValidator.hpp"
#include <stdexcept>
#include <set>
#include <unistd.h>
#include <pwd.h>

ConfigValidator::ConfigValidator(const MainConfig& config, const std::string& project_root)
    : config(config), project_root(project_root) {}

void ConfigValidator::validate()
{
	std::vector<std::string> errors;
    for (size_t i = 0; i < config.servers.size(); ++i)
    {
		config.servers[i]->valid = true;
		try
		{
			validateServerConfig(*config.servers[i]);
		}
   		catch (const std::runtime_error& e)
		{
            errors.push_back(e.what());
			config.servers[i]->valid = false;			
		}
	}
    if (!errors.empty())
	{
        for (size_t i = 0; i < errors.size(); ++i) {
            std::cerr << "Error: " << errors[i] << std::endl;
        }

        if (errors.size() == config.servers.size()) {
            throw std::runtime_error("All server blocks are invalid");
        }
    }
		
}

void ConfigValidator::validateServerConfig(const ServerConfig& server)
{
    checkEssentialDirectives(server);

    validatePort(server.listen_port);
    validateHost(server.host);
    validateClientMaxBodySize(server.client_max_body_size);

    std::map<std::string, LocationConfig>::const_iterator it;
    for (it = server.locations.begin(); it != server.locations.end(); ++it) {
        validateLocationConfig(it->second, project_root);
    }
}

void ConfigValidator::checkEssentialDirectives(const ServerConfig& server)
{
	if (server.listen_port.empty())
        throw std::runtime_error("Missing 'listen' directive in server block");

	if (server.locations.empty())
		throw std::runtime_error("No locations found in server block");
}

void ConfigValidator::validatePort(const std::string& port)
{
    if (port.empty() || port.size() > 5)
        throw std::runtime_error("Invalid port: " + port);
    for (size_t i = 0; i < port.size(); ++i) {
        if (!isdigit(port[i]))
            throw std::runtime_error("Invalid port: " + port);
    }
    int port_num = atoi(port.c_str());
    if (port_num < 1 || port_num > 65535)
        throw std::runtime_error("Invalid port: " + port);
}

void ConfigValidator::validateHost(const std::string& host)
{
    if (!isValidIPAddress(host) && !isValidDomainName(host))
        throw std::runtime_error("Invalid host: " + host);
}

bool ConfigValidator::isValidIPAddress(const std::string& ip)
{
	int num, dots = 0;
	char *ptr;

	if (ip.length() > 15 || ip.empty())
		return false;

	char *str = strdup(ip.c_str());
	ptr = strtok(str, ".");
	if (ptr == NULL)
		{delete str; return false;}
	while (ptr)
	{
		if (!isdigit(*ptr))
			{delete str; return false;}
		num = atoi(ptr);
		if (num < 0 || num > 255)
			{delete str; return false;}
		ptr = strtok(NULL, ".");
		if (ptr != NULL)
			dots++;
	}
	delete str;
	if (dots != 3)
		return false;
	return true;
}
bool ConfigValidator::isValidDomainName(const std::string& domain)
{
	if (domain.empty())
		return false;
	
    if (domain == "localhost")
        return true;

	const std::string validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-";
	
	for (size_t i = 0; i < domain.length(); ++i)
	{
		if (validChars.find(domain[i]) == std::string::npos)
			return false;
	}

	if (domain.find("..") != std::string::npos || domain.front() == '.' || domain.back() == '.')
			return false;

	if (domain.find(".-") != std::string::npos || domain.find("-.") != std::string::npos)
		return false;
	
	size_t lastDotPos = domain.rfind('.');
    if (lastDotPos == std::string::npos || lastDotPos == 0 || lastDotPos == (domain.size() - 1))
        return false;

    return true;

}

void ConfigValidator::validateClientMaxBodySize(const std::string& size)
{
    if (size.empty())
        return;

    size_t result = 0;
    std::string::const_iterator it;
    for (it = size.begin(); it != size.end(); ++it) {
        if (!isdigit(*it))
            throw std::runtime_error("Invalid client_max_body_size, should contain only digits: " + size);

        size_t digit = *it - '0';
        
        if (result > (ULONG_MAX - digit) / 10) {
            throw std::runtime_error("Value overflow detected in client_max_body_size: " + size);
        }

        result = result * 10 + digit;
    }
}

void ConfigValidator::validateLocationConfig(const LocationConfig& location, const std::string& project_root)
{
	validateRoot(location.root, project_root);
	validateAllowedMethods(location.allowed_methods);
	validateUploadStore(location.upload_store, location.root, project_root);
	validateCGI(location.add_cgi, location.cgi_path, location.cgi_allowed_methods, location.root, project_root);
}

void ConfigValidator::validateRoot(const std::string& root, const std::string& project_root)
{
	(void)project_root;
	if (root.empty())
		throw std::runtime_error("Missing 'root' directive in location block");

	std::string path = root;
	if (access(path.c_str(), F_OK) == -1)
		throw std::runtime_error("Root path does not exist: " + path);
}

void ConfigValidator::validateCGI(std::vector<std::string> add_cgi, std::string cgi_path, std::vector<std::string> cgi_allowed_methods, const std::string& location_root, const std::string& project_root)
{
	(void)project_root;
	if (!add_cgi.empty())
	{
		std::vector<std::string>::const_iterator it;
		for (it = add_cgi.begin(); it != add_cgi.end(); ++it)
		{
			if ((*it)[0] != '.')
				throw std::runtime_error("Invalid CGI extension: " + *it);
		}
		if (cgi_path.empty())
			throw std::runtime_error("Missing 'cgi_path' directive in location block");
			
		std::string path = location_root + "/" + cgi_path;
		if (access(path.c_str(), F_OK) == -1)
			throw std::runtime_error("CGI path does not exist: " + path);
		validateAllowedMethods(cgi_allowed_methods);
	}
}

void ConfigValidator::validateUploadStore(const std::string& upload_store, const std::string& location_root, const std::string& project_root)
{
	(void)project_root;
	if (!upload_store.empty())
	{		
		std::string path = location_root + "/" + upload_store;
		if (access(path.c_str(), F_OK) == -1)
			throw std::runtime_error("Upload store path does not exist: " + path);
	}
}

void ConfigValidator::validateAllowedMethods(const std::vector<std::string>& methods)
{
	if (methods.empty())
		throw std::runtime_error("No allowed methods specified");
    std::vector<std::string>::const_iterator it;
    for (it = methods.begin(); it != methods.end(); ++it)
	{
        if (*it != "GET" && *it != "POST" && *it != "DELETE")
        if (*it != "GET" && *it != "POST" && *it != "DELETE")
            throw std::runtime_error("Invalid allowed method: " + *it);
    }
}
