#ifndef __SOCKETS_HPP__
# define __SOCKETS_HPP__

# include <map>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <netdb.h>
# include <unistd.h>
# include <sstream>
# include <fcntl.h>
# include <iostream>
# include <iomanip>
# include <cstdlib>
# include <ctime>
# include "KQueue.hpp"
# include "util.h"
# include "sys/un.h"
# include "ConfigStructures.hpp"
# include "ConfigValidator.hpp"
# include "Tokenizer.hpp"
# include "Parser.hpp"
# include <signal.h> 

# define FILE_READ_BUFFER_SIZE	1000
# define CRLF			"\r\n"
# define CLR_TERM			"\e[1;1H\e[2J"
# ifndef PROJECT_PATH
#  define PROJECT_PATH "/Users/hsobane/projects/teamWeb/"
# endif
# ifndef DEBUG
#  define DEBUG 0
# endif
# define CONFIG_PATH PROJECT_PATH"config"
# define CGI_COMM CONFIG_PATH"/cgi_comm"
# define SOCKETS_PATH CONFIG_PATH"/sockets"
# define SERVER_ALL_ROOT PROJECT_PATH"www"
# define DEFAULT_CONFIG CONFIG_PATH"/config_files/default.conf"
# define _S_DEL "__S_"CRLF"_DEL__"
# define _M_DEL "__M_"CRLF"_DEL__"
# define CGI_TIME_LIMIT		10	// s
# define CGI_PIPE_MAX_SIZE	1000
# define UNIX_SOCK_BUFFER	8000
# define PYTHON_PATH	"/usr/local/bin/python3"
# define PHP_PATH		"/Users/hsobane/php-cgi"
# define PERL_PATH		"/usr/bin/perl"
# define JAVA_PATH		"/usr/bin/java"
# define JS_PATH		"/usr/local/bin/node"
# define SHELL_PATH		"/bin/bash"

# define KNRM  "\x1B[0m"
# define KRED  "\x1B[31m"
# define KGRN  "\x1B[32m"
# define KYEL  "\x1B[33m"
# define KBLU  "\x1B[34m"
# define KAG  "\x1B[35m"
# define KCYN  "\x1B[36m"
# define KWHT  "\x1B[37m"
# define KUND  "\033[4m"
# define KBGR  "\033[7m"

class	Response;
class	Request;
struct	ServerConfig;
struct	MainConfig;
struct	LocationConfig;

typedef std::vector<ServerConfig *>::iterator mit;

class Sockets {
public:
	Sockets();
	~Sockets();
	Sockets(const Sockets &);
	Sockets &operator = (const Sockets &);

	void						run();
	void						startServers();
	void						accept(int);
	void						recvFrom(int);
	void						sendTo(int);
	void						closeConn(int);
	void						resetConn(int);
	void						cleanUp();
	void						kqueueLoop();
	std::string					get_mime_type(std::string);
	//
	void							initiate_servers(MainConfig&, char **);
	int							initiate_master_process(std::pair<int, int>*, std::string, std::string, std::string, std::string);
	bool							update_master_state();
	std::string						execute_script(std::string, std::string);
	void							check_and_remove(std::string);
	void							_enrg_env_var(std::string, std::string);
	void							_initiate_env_variables(char**);
	std::string						format_env();
	bool							is_valid_mime(std::string);
	bool							cgi_in(int, std::pair<Request, Response>*, ServerConfig*);
	void							cgi_out(int);
	void							update_cgi_state(struct kevent&);
	std::map<std::string, std::string>				env_variables;
private:
	MIME						_mime;
	MainConfig					_main_config;
	KQueue						_kqueue;
	std::map<int, ServerConfig *>				_fd_to_server;
	std::map<std::string, ServerConfig *>			_dup_servers;
	//< unix_process, < worker_pid, client > >
	std::map<int, std::pair<int, int>*>			_cgi_clients;
	//
	pid_t						master_PID;
	int						master_process;
	int						cgi_controller;
	std::string					socket_path;
	bool						active_master;
};

void		fix_up_signals(void (*)(int));
std::string	clean_up_stuff(std::string, std::string, std::string);
std::string	_generate_random_string(std::string, int);

# include "Response.hpp"
# include "Request.hpp"
# include "ConfigStructures.hpp"

#endif
