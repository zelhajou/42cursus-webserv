#include "Sockets.hpp"

Sockets::Sockets(const Sockets &S) {*this = S; this->_main_proc = true;}
Sockets	&Sockets::operator = (const Sockets &S) { (void)S; return *this;}

void	fix_up_signals(void (*f)(int)) {
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, f);
	signal(SIGQUIT, f);
	signal(SIGINT, f);
}

static	void	child_ex(int sig_num) {
	std::cout << KCYN"master_process:" << KNRM << " received signal(" << KGRN << sig_num
		<< KNRM << ") exiting..\n";
	exit(sig_num);
}

Sockets::Sockets( void ) : active_master(0), _main_proc(true) {
	std::srand(std::time(NULL));
	std::cout << CLR_TERM;
}

Sockets::~Sockets() {
	if (DEBUG && this->_main_proc) {
		std::cout << KRED"cleaning...\n";
		std::cout << "deleting unix socket: " << KNRM
			<< this->socket_path << std::endl;
	}
	this->check_and_remove(this->socket_path);
	if (this->active_master && kill(this->active_master, 0) == 0) {
		if (DEBUG && this->_main_proc)
			std::cout << KRED"killing master process\n" << KNRM;
		close(this->master_process);
		kill(this->master_PID, SIGKILL);
	}
	if (DEBUG && this->_main_proc)
		std::cout << KRED"closing active connections..\n" << KNRM;

	std::set<ServerConfig*>	servers;
	for (std::map<int, ServerConfig*>::iterator i=this->_fd_to_server.begin(); i!=this->_fd_to_server.end();++i) {
			close(i->first);
			servers.insert(i->second);
	}
	for(std::set<ServerConfig*>::iterator i=servers.begin(); i!=servers.end();++i) {
		if (DEBUG && this->_main_proc)
			std::cout << KRED << "deleting: " << (*i)->server_name << KNRM << std::endl;
		for (std::map<int, std::pair<Request, Response>* >::iterator j=(*i)->_requests.begin(); j!=(*i)->_requests.end();++j) {
			delete j->second;
		}
		delete *i;
	}
	for (std::map<int, std::pair<int, int>*>::iterator i=this->_cgi_clients.begin(); i!=this->_cgi_clients.end();++i) {
		close(i->first);
		delete i->second;
	}
}

std::string conc_urls(std::string s_1, std::string s_2) {
	int	s_1_p = s_1.rfind("/"), s_2_p = s_2.find("/"),
		con_1 = s_1_p == std::string::npos || s_1_p != s_1.size() - 1,
		con_2 = s_2_p == std::string::npos || s_2_p != 0;
	s_2 = s_2.substr((!con_1 && !con_2) ? 1 : 0);
	return   s_1 + (con_1 && con_2 ? "/" : "") + s_2;
}

///////////////////////////	CGI

static	std::string	exec_job(pid_t *grandchild, char *executer, char *script, char **env, std::string file_name) {
	std::string	output;
	char		buffer[CGI_PIPE_MAX_SIZE];
	int		pi[2], s;
	if (pipe(pi) < 0 || (*grandchild = fork()) < 0)
		return "webserv_cgi_status=500; CGI pipe()/fork() failure";
	else if (!*grandchild) {
		char	*argv[] = {executer, script, 0};
		if (file_name.size()) {
			int fd = open(file_name.c_str(), R_OK);
			if (fd < 0)	exit(EXIT_FAILURE);
			dup2(fd, 0); close(fd);
		}
		dup2(pi[1], 1); close(pi[1]);
		execve(argv[0], argv, env);
		for (int i=0; env[i]; i++)
			{ delete env[i]; }
		delete	[] env;
		std::cout << KRED << "execve: " << strerror(errno) << KNRM << "\n";
		exit(EXIT_FAILURE);
	}
	close(pi[1]);
	fd_set	r_set, r_copy; FD_ZERO(&r_set);
	FD_ZERO(&r_copy); FD_SET(pi[0], &r_set);
	struct	timeval	tv; std::memset(&tv, 0, sizeof(tv));
	tv.tv_sec = CGI_TIME_LIMIT;
	output.append("webserv_cgi_status=200;");
	while (true) {
		r_copy = r_set;
		s = select(pi[0] + 1, &r_copy, NULL, NULL, &tv);
		if (s < 0) {kill(*grandchild, SIGINT); output = "webserv_cgi_status=500; cgi ERROR"; break ;}
		else if (s > 0 && FD_ISSET(pi[0], &r_copy)) {
			std::memset(buffer, 0, sizeof(buffer));
			s = read(pi[0], buffer, CGI_PIPE_MAX_SIZE -1);
				if (s <= 0) break ;
			output.append(buffer, s);
		}
		else	{kill(*grandchild, SIGINT); output = "webserv_cgi_status=504; cgi TIMEOUT"; break ;}
	}
	close(pi[0]);
	std::remove(file_name.c_str());
	return	output;
}

static	int	geta_unix_socket(struct sockaddr_un &address, std::string socket_path) {
	std::memset(&address, 0, sizeof(address));
	int	sock;
	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)	return -1;
	address.sun_family = AF_UNIX;
	std::strcpy(address.sun_path, socket_path.c_str());
	return		sock;
}

static	char **prepare_env(std::string s_env, std::string semi_del) {
	std::vector<std::string>	v_env;
	size_t			pos;
	char			**env;

	for (;(pos = s_env.find(semi_del)) != std::string::npos;) {
		v_env.push_back(s_env.substr(0, pos));
		s_env = s_env.substr(pos + semi_del.size()); }
	pos = 0;
	env = new char*[ v_env.size() + 1 ];
	std::vector<std::string>::iterator	it = v_env.begin();
	for (; it!=v_env.end();++it, pos++) {
		env[pos] = new char[ it->size() +1 ];
		std::strcpy(env[pos], it->c_str());}
	env[pos] = 0;
	return	env;
}

static	void	master_routine(std::string socket_path, std::string executer, std::string script, std::string file_name, std::string s_env) {
	fix_up_signals(child_ex);
	std::string	output;
	struct	sockaddr_un address;
	pid_t	grandchild;
	int	sock, n, s;
	fd_set	w_set;	FD_ZERO(&w_set);
	if ((sock = geta_unix_socket(address, socket_path)) < 0)	exit(20);
	for (int i=15; i; i--) {
		if (connect(sock, (struct sockaddr*)&address, sizeof(address)) == 0)	break;
		else if (i == 1) exit(21); }
	char	**env = prepare_env(s_env, _S_DEL);
	output = exec_job(&grandchild, const_cast<char*>(executer.c_str()),
		const_cast<char*>(script.c_str()),
		env,
		file_name);
	waitpid(grandchild, &s, 0);
	for (int i=0; env[i]; i++)
	{ delete env[i]; }
	delete	[] env;
	FD_SET(sock, &w_set);
	n = select(sock + 1, NULL, &w_set, NULL, NULL);
	if (n < 0) exit(22);
	if (n > 0 && FD_ISSET(sock, &w_set)) {
		while (true) {	//	-->
			n = send(sock, output.c_str(), output.size(), 0/*MSG_DONTWAIT*/);
			if (n <= 0)	break;
			output = output.substr( n );
		}
		exit(WEXITSTATUS(s));
	}
}

void	Sockets::_enrg_env_var(std::string name, std::string value) {
	this->env_variables[ name ] = value;
}

std::string	Sockets::format_env() {
	std::string	output;
	std::map<std::string, std::string>::iterator	i = this->env_variables.begin();
	for (; i!=this->env_variables.end(); ++i)
		output.append(i->first + "=" + i->second + _S_DEL);
	return	output;
}

void	Sockets::_initiate_env_variables(char **env) {
	int		size(0);
	size_t	pos = 0;

	std::string	f_half, s_half;
	if (env)	for (;env[size]; size++);
	if (size) {
		for (int i=0; i<size; i++) {
			std::string	temp(env[i]);
			pos = temp.find("=");
			if (pos == temp.npos) continue;
			f_half = temp.substr(0, pos);
			s_half = temp.substr(pos + 1);
			if (f_half.empty() || s_half.empty()) continue;
			this->env_variables[ f_half ] = s_half;
		}
	}
}

void	Sockets::initiate_servers(MainConfig &main_config, char **env) {
	this->_main_config = main_config;
	this->_initiate_env_variables(env);
}

void	Sockets::check_and_remove(std::string target) {
	struct	stat	output;
	if (stat(target.c_str(), &output) != 0)	return;
	std::remove(target.c_str());
}

static	std::string	get_executer(std::pair<std::string, std::string> cgi_info, std::string uri) {
	//if (cgi_info.first.empty() || !access(uri.c_str(), X_OK))	return	uri;
	if (cgi_info.first == ".py")				return	PYTHON_PATH;
	if (cgi_info.first == ".php")				return	PHP_PATH;
	if (cgi_info.first == ".pl" || cgi_info.first == ".pm")	return	PERL_PATH;
	if (cgi_info.first == ".java" || cgi_info.first == ".jvs")		return	JAVA_PATH;
	if (cgi_info.first == ".js" || cgi_info.first == ".javascript")	return	JS_PATH;
	if (cgi_info.first == ".sh")					return	SHELL_PATH;
	return	uri;
}

int	Sockets::initiate_master_process(std::pair<int, int>* pr_info, std::string exec, std::string uri, std::string file, std::string env)
{
	struct	sockaddr_un	address;
	int			unix_listener, unix_sock, pid;
	this->socket_path = conc_urls(SOCKETS_PATH, "unix_quick_private_socket");
	this->check_and_remove(this->socket_path);
	if ((unix_listener = geta_unix_socket(address, this->socket_path)) < 0) return -1;
	if (bind(unix_listener, (struct sockaddr*)&address, sizeof(address)) < 0) return -1;
	if (listen(unix_listener, 5) < 0) return -1;
	if ((pid = fork()) < 0) return -1;
	if (pid == 0) {
		this->_main_proc = false;
		master_routine(this->socket_path, exec, uri, file, env);
		exit(EXIT_FAILURE);
	}
	if (pid == 0)	{delete pr_info; this->_main_proc = false; master_routine(this->socket_path, exec, uri, file, env);}
	this->_main_proc = true;
	if ((unix_sock = ::accept(unix_listener, NULL, NULL)) < 0)  return -1;
	close(unix_listener);
	pr_info->first = pid;
	return	unix_sock;
}

static	std::string	_conc_(std::string input, char c) {
	try {
		int	i(0);
		while (input[i] == c)
			input = input.substr(1);
		i = input.size() - 1;
		while (input[i] == c) {
			input = input.substr(0, i - 1);
			i = input.size() - 1;
		}
	return	input;
	}
	catch (std::exception &l)
	{
		return	input;
	}
}

static	std::string	_cgi_header(std::string input, std::string v_name, std::string v_name_2) {
		std::string		temp_c_t;
		size_t	pos = input.find(v_name);
		if (pos == std::string::npos) pos = input.find(v_name_2);
		if (pos != std::string::npos) {
			temp_c_t = input.substr(pos + v_name.size());
			pos = temp_c_t.find("\n");
			if (pos != std::string::npos) {
				temp_c_t = _conc_(temp_c_t.substr(0, pos), ' ');
				return	temp_c_t;
			}
		}
		return	"";
}

bool	Sockets::cgi_in( int client, std::pair<Request, Response>* pai, ServerConfig* server) {
	if (!pai->second._request->_cgi_info.second.empty()) {
		size_t    pos = pai->second._request->_request.first_line.uri.rfind(pai->second._request->_cgi_info.second);
		if (pos != std::string::npos)
			pai->second._request->_request.first_line.uri = pai->second._request->_request.first_line.uri.substr(0, pos);
	}
	if (access(pai->second._request->get_first_line().uri.c_str(), R_OK) < 0) {
		pai->second.target_file = pai->second.generate_status_file(NOT_FOUND, server, "CGI script NOT FOUND");
		return	false;
	}
	std::string	input, to_stdin_input, _file_name;
	if (pai->second._request->get_first_line().method == "GET" && pai->second._request->_query_string.size()) {
		for (std::vector<std::pair<std::string, std::string> >::iterator it = pai->second._request->_query_string.begin();
			it != pai->second._request->_query_string.end(); ++it)
			input.append(it->first + "=" + it->second + "&");
	}
	else if (pai->second._request->get_first_line().method == "POST" && pai->second._request->get_headers().content_type.find("multipart/form-data") != std::string::npos)
		input.append(pai->second._request->_request.body);
	else	input.append(pai->second._request->_request.raw_body);
	_enrg_env_var("CONTENT_LENGTH", std::to_string(input.size()));
	_enrg_env_var("REQUEST_METHOD", pai->second._request->get_first_line().method);
	_enrg_env_var("CONTENT_TYPE", pai->second._request->get_headers().content_type);
	_enrg_env_var("SERVER_NAME", server->server_name);
	_enrg_env_var("SERVER_PORT", server->listen_port);
	_enrg_env_var("SERVER_PROTOCOL", "HTTP/1.1");
	_enrg_env_var("HTTP_COOKIE", pai->second._request->get_headers().cookie);
	std::string	uri = pai->second._request->get_first_line().uri;
	if (!pai->second._request->_cgi_info.second.empty()) {
		_enrg_env_var("PATH_INFO", pai->second._request->_cgi_info.second);
		if (DEBUG)
			std::cout << "\t" << KCYN"PATH_INFO: " << KNRM << pai->second._request->_cgi_info.second << std::endl;
		uri = uri.substr(0, uri.find(pai->second._request->_cgi_info.second));
	}	else	_enrg_env_var("PATH_INFO", "");
	_enrg_env_var("REQUEST_URI", uri);
	if (pai->second._request->get_first_line().method == "GET")	_enrg_env_var("QUERY_STRING", input);
	else	{ _enrg_env_var("QUERY_STRING", ""); to_stdin_input = input; }
	try {
		if (!to_stdin_input.empty()) {
			_file_name = conc_urls(CGI_COMM, _generate_random_string(uri, 20));
			std::fstream	_cgi_file(_file_name, std::ios::out|std::ios::binary);
			if (!_cgi_file.is_open())	throw std::runtime_error("can\'t setup comm. medium with cgi");
			_cgi_file << to_stdin_input;
			_cgi_file.close();
		}
		int			unix_sock;
		std::pair<int, int>	 	*pr_info = new std::pair<int, int>;
		if ((unix_sock = this->initiate_master_process(pr_info, get_executer(pai->second._request->_cgi_info, uri),
				uri, _file_name, this->format_env())) < 0)
			{delete pr_info; throw std::runtime_error("can\'t create master process");}
		std::memset(&this->_kqueue.event, 0, sizeof(this->_kqueue.event));
		EV_SET(&this->_kqueue.event, unix_sock, EVFILT_READ, EV_ADD|EV_ENABLE, 0, 0, (void *)"unix");
		kevent(this->_kqueue.kq, &this->_kqueue.event, 1, NULL, 0, NULL);
		std::memset(&this->_kqueue.event, 0, sizeof(this->_kqueue.event));
		EV_SET(&this->_kqueue.event, pr_info->first, EVFILT_PROC, EV_ADD|EV_ENABLE, NOTE_EXIT, 0, (void *)"pid");
		kevent(this->_kqueue.kq, &this->_kqueue.event, 1, NULL, 0, NULL);
		this->_kqueue.current_events += 2;
		pr_info->second = client;
		this->_cgi_clients[ unix_sock ] = pr_info;
	} catch (std::exception &l) {
		std::cout << KRED << "\tSockets::cgi_in(): just catched:" << l.what() << KNRM << std::endl;
		pai->second.target_file = pai->second.generate_status_file(INTERNAL_SERVER_ERROR, server, l.what());
		return	false;
	}
	return	true;
}

void	Sockets::cgi_out(struct kevent &event) {
	int	unix_sock = event.ident;
	std::map<int, std::pair<int, int> *>::iterator		it = this->_cgi_clients.find( unix_sock );
	int						client = it->second->second;
	ServerConfig*					server = this->_fd_to_server.find(client)->second;
	std::map<int, std::pair<Request, Response> *>::iterator	pai = server->_requests.find(client);
	std::string		out_file;
	if (event.flags & EV_ERROR || event.fflags & EV_ERROR
		|| event.flags & EV_EOF || event.fflags & EV_EOF) 
		out_file = pai->second->second.generate_status_file(INTERNAL_SERVER_ERROR, server, "cgi stuff");
	else {
		out_file = conc_urls(CGI_COMM, _generate_random_string(pai->second->second._request->get_headers().host, 15)) + ".html";
		std::fstream	_file(out_file, std::ios::out);
		if (_file.is_open()) {
			int		n(0);
			std::string	output;
			char		buffer[1000];
			for (;;) {
				std::memset(buffer, 0, sizeof(buffer));
				n = recv(unix_sock, buffer, 1000, MSG_DONTWAIT);
				if (n <= 0)	break;
				output.append(buffer, n);
			}
			size_t pos = output.find("webserv_cgi_status=");
			int cgi_status = std::atoi(output.substr(pos+19, pos+22).c_str());
			output = output.substr(pos+23);
			if (cgi_status != 200)	out_file = pai->second->second.generate_status_file((e_status)cgi_status, server, output);
			else {
				try {
					std::string	temp_c_t = _cgi_header(output, "Content-type:", "Content-Type:");
					if (temp_c_t.empty()) temp_c_t = _cgi_header(output, "content-type:", "CONTENT-TYPE:");
					if (!temp_c_t.empty() && this->is_valid_mime(temp_c_t))
						pai->second->second._file_type = temp_c_t;
					temp_c_t = _cgi_header(output, "set-cookie:", "Set-cookie:");
					if (temp_c_t.empty()) temp_c_t = _cgi_header(output, "Set-Cookie:", "SET-COOKIE:");
					if (!temp_c_t.empty()) {
						pai->second->second._cgi_cookie = temp_c_t;
						pai->second->second._has_cookies = true; }
					temp_c_t = _cgi_header(output, "Location:", "location:");
					if (!temp_c_t.empty()) {
						pai->second->second._cgi_redir = temp_c_t;
						pai->second->second._has_redir = true;
						pai->second->second._response_status = TEMP_REDIRECT; }
					pos = output.find("\r\n\r\n");
					if (pos != std::string::npos)	output = output.substr(pos + 4);
					else {
						pos = output.find("\n\n");
						if (pos != std::string::npos) output = output.substr(pos + 2);
						else	throw	std::runtime_error("invalid cgi output formatting");
					}
				} catch (std::exception &l) {
					std::cout << KRED << "\tResponse::process_cgi_exec(): just catched:" << l.what() << KNRM << std::endl;
					out_file = pai->second->second.generate_status_file(INTERNAL_SERVER_ERROR, server, l.what());
				}
			}
			_file << output;
			_file.close();
			
		}
		else out_file = pai->second->second.generate_status_file(INTERNAL_SERVER_ERROR, server, "can\'t create cgi file");
	}
	pai->second->second.target_file = out_file;
	this->_kqueue.SET_QUEUE(unix_sock, EVFILT_READ, 0);
	close(unix_sock);
}

void	Sockets::update_cgi_state(struct kevent &event) {
	std::map<int, std::pair<int, int> *>::iterator it = this->_cgi_clients.begin();
	for (; it != this->_cgi_clients.end(); it++)
		if (it->second && it->second->first == static_cast<int>(event.ident)) {
			int	status;
			this->_kqueue.SET_QUEUE(it->second->second, EVFILT_WRITE, 1);
			waitpid(event.ident, &status, 0);
			ServerConfig	*server = this->_fd_to_server.find(it->second->second)->second;
			std::map<int, std::pair<Request, Response> *>::iterator pai = server->_requests.find(it->second->second);
			pai->second->second._begin_response(*this, server, WEXITSTATUS(status));
			delete	it->second;
			this->_cgi_clients.erase( it );
			break ;
		}
	std::memset(&this->_kqueue.event, 0, sizeof(this->_kqueue.event));
	EV_SET(&this->_kqueue.event, event.ident, EVFILT_PROC, EV_DELETE, 0, 0, 0);
	kevent(this->_kqueue.kq, &this->_kqueue.event, 1, NULL, 0, NULL);
	this->_kqueue.current_events -= 1;
}

///////////////////////////

void	Sockets::accept(int sock_fd) {
	struct	sockaddr	address;
	socklen_t	si = sizeof(address);
	int new_s_fd = ::accept(sock_fd, &address, &si);
	if (new_s_fd < 0)	return ;
	ServerConfig		*target = this->_fd_to_server.find(sock_fd)->second;
	this->_fd_to_server[ new_s_fd ] = target;
	this->_kqueue.SET_QUEUE(new_s_fd, EVFILT_READ, 1);
	if (DEBUG)
		std::cout << "\t  [" << target->server_name << std::setw(15-target->server_name.size())
			<< ": Accept connection: " << KGRN << new_s_fd << KNRM << "]" << std::endl;
}

void	Sockets::recvFrom(int sock_fd) {
	ServerConfig *serv = this->_fd_to_server.find(sock_fd)->second;
	std::map<int, std::pair<Request, Response> *>::iterator	pai = serv->_requests.find(sock_fd);
	if (pai == serv->_requests.end()) {
		std::vector<ServerConfig*>	servs;
		servs.push_back(serv);
		if (serv->is_duplicated)
			for (std::map<std::string, ServerConfig*>::iterator it = this->_dup_servers.begin(); it != this->_dup_servers.end(); ++it) 
				if (it->first == serv->host+":"+serv->listen_port)	servs.push_back(it->second);
		std::pair<Request, Response>	*new_pair = new std::pair<Request, Response>;
		serv->_requests[ sock_fd ] = new_pair;
		serv->_requests[ sock_fd ]->first.set_servers( servs );
		serv->_requests[ sock_fd ]->first.set_fd( sock_fd );
		this->recvFrom( sock_fd );
		return ;
	}
	pai->second->first.recvRequest();
	if (pai->second->first.getState() == DONE || pai->second->first.getState() == ERROR) {
		char buffer[BUFFER_SIZE];
		int	b;
		while ((b = recv(sock_fd, buffer, BUFFER_SIZE, MSG_DONTWAIT)) > 0 && b != -1); 
		this->_kqueue.SET_QUEUE(sock_fd, EVFILT_READ, 0);
		pai->second->second._request = &pai->second->first;
		pai->second->second._response_status = pai->second->first.getStatus();
		if (pai->second->first.getStatus() == OK
			&& !pai->second->first._is_return
			&& pai->second->first._location_type == CGI) {
			if (this->cgi_in( sock_fd, pai->second, serv ))
				return ;
			else	pai->second->second._begin_response(*this, serv, 0);
		}
		else	pai->second->second._initiate_response(sock_fd, *this, serv);
		this->_kqueue.SET_QUEUE(sock_fd, EVFILT_WRITE, 1);
	}
}

void	Sockets::sendTo(int sock_fd) {
	ServerConfig *serv = this->_fd_to_server.find(sock_fd)->second;
	std::map<int, std::pair<Request, Response>* >::iterator pai = serv->_requests.find(sock_fd);
	pai->second->second.sendResponse(sock_fd, *this, serv);
	if (pai->second->second.get_status() == DONE) {
		if (pai->second->second._connection_type == "keep-alive" && pai->second->second._has_body) {
			this->_kqueue.SET_QUEUE(sock_fd, EVFILT_WRITE, 0);
			this->_kqueue.SET_QUEUE(sock_fd, EVFILT_READ, 1);
			this->resetConn(sock_fd);
		}
		else	this->closeConn(sock_fd);
	}
}

void	Sockets::closeConn(int sock_fd) {
	std::map<int, std::pair<int, int>* >::iterator it = this->_cgi_clients.begin();
	for (; it != this->_cgi_clients.end(); ++it) {
		if (it->second->second == sock_fd) {
			kill(it->second->first, SIGINT);
			close(it->first);
		}
	}
	//
	this->resetConn(sock_fd);
	std::map<int, ServerConfig*>::iterator i = this->_fd_to_server.find(sock_fd);
	if (i != this->_fd_to_server.end()) {
		if (DEBUG)
			std::cout << "\t  [" << i->second->server_name << std::setw(15-i->second->server_name.size()) << ": Closed connection: " << KRED << sock_fd << KNRM
				<< " , remaining sockets: " << KBGR " "<< this->_kqueue.get_current_events() << " " << KNRM << "]\n";
		this->_fd_to_server.erase(sock_fd);
	}
	this->_kqueue.SET_QUEUE(sock_fd, 0, 0);
	close(sock_fd);
}

void	Sockets::resetConn(int sock_fd) {
	this->_fd_to_server[ sock_fd ]->closeConn(sock_fd);
}

void	Sockets::cleanUp() {} // TODO

///////

std::string	clean_up_stuff(std::string input, std::string garbage, std::string target) {
	if (input.empty()||garbage.empty()||garbage.size() != target.size())	return "";
	size_t				pos;
	for (size_t i=0; i < garbage.size(); i++)
	{
		std::string	shgarbage(1, garbage[i]);
		while (true) {
			pos = input.find(shgarbage);
			if (pos == input.npos)	break;
			std::string	sec_tar(1, target[i]);
			input.replace(pos, 1, sec_tar);
		}
	}
	return	input;
}

std::string	_generate_random_string(std::string seed, int length) {
	std::string	output = seed;
	for (int i=0;i<length;i++) {
		std::string    r(1, static_cast<char>(std::rand() % (122 - 48) + 48 ));
		output.append( r );
	}
	return	clean_up_stuff(output, "[\\]^`:;<>=?/ ", "_____________");
}

//////

void	Sockets::kqueueLoop() {
	std::map<int, ServerConfig*>::iterator		it;
	std::map<int, std::pair<int, int>*>::iterator	iit;
	int					n;
	for (;;)
	{
		struct	kevent	events[ this->_kqueue.get_current_events() ];
		n = this->_kqueue.CHECK_QUEUE(events);
		if (n > 0)
			for (int i=0; i < n; i++)
			{
				if (!events[i].udata && (events[i].flags & EV_ERROR || events[i].fflags & EV_ERROR
					|| events[i].flags & EV_EOF || events[i].fflags & EV_EOF)) {
					if (DEBUG) std::cout << KRED << "\t  [err/eof on: " << events[i].ident << "]" << KNRM << std::endl;
					this->closeConn(events[i].ident);
				}
				else if (events[i].filter == EVFILT_READ) {
					if ((it = this->_fd_to_server.find(events[i].ident)) != this->_fd_to_server.end() && it->first == it->second->_socket) 
						this->accept(events[i].ident);
					else if ((iit = this->_cgi_clients.find(events[i].ident)) != this->_cgi_clients.end()) 
						this->cgi_out(events[i]);
					else 
						this->recvFrom(events[i].ident);
				}
				else if (events[i].filter == EVFILT_WRITE) 
					this->sendTo(events[i].ident);
				else if (events[i].filter == EVFILT_PROC) 
					this->update_cgi_state(events[i]);
			}
	}
}

static void initAddInfo(struct addrinfo& ai) {
	memset(&ai, 0, sizeof(struct addrinfo));
	ai.ai_family = AF_UNSPEC;
	ai.ai_socktype = SOCK_STREAM;
	ai.ai_flags = AI_PASSIVE;
}

static int getAddrInfo(struct addrinfo *hints, struct addrinfo **res, mit it) {
	int					rval;

	if ((rval = getaddrinfo((*it)->host.c_str(), (*it)->listen_port.c_str(), hints, res)) != 0) {
		std::cerr << KYEL << "server: " << (*it)->host << ":" << (*it)->listen_port << " is enable to start." << KNRM << std::endl;
		return (-1);
	}
	return (0);
}

static int createSocket(struct addrinfo *res, mit it) {
	struct addrinfo		*tmp;
	int					sock;
	int					yes = 1;

	(void)it;
	tmp = res;
	while (tmp) {
		sock = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
		if (sock > 0) {
			if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) >= 0) {
				if (bind(sock, tmp->ai_addr, tmp->ai_addrlen) >= 0)
					return (sock);
			}
			close(sock);
		}
		else if (!tmp->ai_next)	return (0);
		tmp = tmp->ai_next;
	}
	return (-1);
}

void	Sockets::startServers() {
	struct addrinfo		hints;
	struct addrinfo		*res;
	int					sock;
	int					nbr = 0;
	bool					this_status(true);
	std::vector<ServerConfig* >			servers = this->_main_config.servers;

	for (mit it = servers.begin(); it != servers.end(); it++) {
		initAddInfo(hints);
		if (getAddrInfo(&hints, &res, it) < 0) continue ;
		if ((sock = createSocket(res, it)) < 0) {
			std::map<int, ServerConfig*>::iterator	i = this->_fd_to_server.begin();
			for(; i != this->_fd_to_server.end(); ++i)
				if (i->second->host == (*it)->host && i->second->listen_port == (*it)->listen_port) {
					this_status = false;
					if (i->second->server_name == (*it)->server_name)
						std::cout << KYEL"warning: " << KNRM << "same host, port, server_name, IGNORING the second\n";
					else {
						this->_dup_servers[i->second->host+":"+i->second->listen_port] = *it;
						i->second->is_duplicated = true;
					}
				}
		}
		else if (sock == 0) {
			std::cerr << KYEL << "server: " << (*it)->host << ":" << (*it)->listen_port << " is enable to start. socket() failure" << KNRM << std::endl;
			continue ;
		}
		freeaddrinfo(res);
		if (!this_status) { this_status = true; continue ; }
		if (listen(sock, 100) < 0)
			{std::cerr << KYEL << "server: " << (*it)->host << ":" << (*it)->listen_port << " is unable to start. listen() failure" << KNRM << std::endl; close(sock); continue; }
		else	std::cout << "\n[" << KGRN << (*it)->server_name << KNRM << ": " << (*it)->host << ":"
			<< (*it)->listen_port << ", " << (*it)->locations.size() << " locations " << KGRN << "STARTED successfully" << KNRM"]" << std::endl;
		(*it)->_socket = sock;
		this->_kqueue.SET_QUEUE(sock, EVFILT_READ, 1);
		this->_fd_to_server[ sock ] = *it;
		nbr++;
	}
	if (nbr == 0) {
		throw std::runtime_error(KRED + std::string("No server started") + KNRM);
	}
}

void	Sockets::run() {
	this->startServers();
	this->kqueueLoop();
}

std::string	Sockets::get_mime_type(std::string ext) { return	this->_mime.getMIME(ext); }
bool		Sockets::is_valid_mime(std::string type) {
	std::map<std::string, std::string>::iterator	i = this->_mime._mime.begin();
	for (; i!=this->_mime._mime.end(); ++i) {
		if (type.find(i->second) != type.npos)	return	true;
	}
	return	false;
}
