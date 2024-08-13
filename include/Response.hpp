#ifndef __RESPONSE_HPP__
# define __RESPONSE_HPP__

# include <string>
# include <vector>
# include <fstream>
# include "util.h"
# include "Request.hpp"
# include <dirent.h>
# include "ConfigStructures.hpp"

class	Sockets;

class	Response {
	public:
		Response();
		Response(const Response &);
		Response	&operator = (const Response &);
		~Response();
		void				_initiate_response(int, Sockets &, ServerConfig*);
		void				_begin_response(Sockets &, ServerConfig*, int);
		void				sendResponse(int, Sockets&, ServerConfig*);
		size_t				form_headers(ServerConfig*);
		e_parser_state			get_status();
		std::string			http_code_msg(e_status);
		size_t				get_file_size();
		bool				_new_session;
		std::string			generate_status_file(e_status, ServerConfig*, std::string);
		std::string			_connection_type;
		std::ifstream			_file;
		std::string			target_file;
		size_t				_file_size;
		std::string			_file_type;
		std::string			_cgi_redir;
		std::string			_cgi_cookie;
		bool				_has_redir;
		bool				_has_cookies;
		std::string			_session_id;
		std::vector<size_t>			_sent;
		//
		void				file_to_disk(int);
		std::fstream			_upload_stream;
		t_post_body			*_form_field;
		std::vector<size_t>			_recv;
		std::string			_upload_path;
		std::string			_upload_target;
		bool				_raw_upload;
		bool				_post_status;
		//
		Request				*_request;
		e_parser_state			status;
		std::string			header;
		e_status				_response_status;
		bool				_has_body;
};

# include "Sockets.hpp"
#endif
