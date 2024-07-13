#ifndef __REQUEST_HPP__
# define __REQUEST_HPP__

# include <string>
# include <map>
# include <fstream>
# include <sys/socket.h>
# include <unistd.h>
# include <sys/stat.h>
# include "util.h"
# include "Location.hpp"
# include "TrieTree.hpp"

class Request {
public:
	Request();
	Request(const Request &);
	Request &operator = (const Request &);
	~Request();

	void				recvRequest(int);
	void				POST();
	void				parse_first_line();
	void				parse_headers();
	void				parse_body();
	void				parse_uri();
	e_parser_state		getState();
	e_status			getStatus();
	void			setStatus(e_status);
	void				check_uri();
	t_first_line		get_first_line();
	t_headers			get_headers();
	e_location_type		get_location_type();
	void			set_locations(std::map<std::string, Location>&);

//private:
	e_parser_state				_state;
	e_status					_status;
	bool						_has_body;
	int							_fd;
	std::ifstream				_file;
	size_t						_recv;
	t_first_line				_first_line;
	t_headers					_headers;
	std::string					_body;
	size_t						_timeout;
	bool						_chunked;
	size_t						_content_length;
	std::vector<LocationNode*>	_location_tree;
	std::string					_return_string;
	e_location_type				_location_type;
};

#endif
