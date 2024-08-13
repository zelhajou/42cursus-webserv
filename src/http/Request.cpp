#include "Request.hpp"

Request::Request() : _fd(-1), _max_body_size(0), _state(FIRST_LINE), _status(OK), _chunk_size(0), _is_return(0) {
	this->_method = MTH_NONE;
	this->_request.status = STATUS_NONE;
	this->_request.state = FIRST_LINE;
	this->_request.headers.content_length = 0;
	this->_request.body.clear();
	this->_request.raw_body.clear();
	this->_location_type = LOC_NONE;
	this->_total_body_size = 0;
}

Request::Request(const Request &R) { *this = R; }
Request	&Request::operator = (const Request &R) { (void)R; return *this; }

Request::~Request() {
	std::vector<LocationNode*>::iterator it;
	for (it = this->_location_tree.begin(); it != this->_location_tree.end(); it++) {
		delete *it;
	}
}

void Request::setLocation() {
	ServerConfig*	server = this->_servers[0];
	std::vector<ServerConfig*>::iterator it = this->_servers.begin();
	for (; it != this->_servers.end(); it++) {
		if ((*it)->server_name == this->_request.headers.host) {
			server = *it;
			break ;
		}
	}
	this->_max_body_size = std::strtoll(server->client_max_body_size.c_str(), NULL, 10);

	std::map<std::string, LocationConfig>::iterator itm;
	for (itm = server->locations.begin(); itm != server->locations.end(); itm++) {
		LocationNode*	node = new LocationNode;
		node->name = itm->first;
		node->location = &itm->second;
		::insert(this->_location_tree, node, ::cmp);
	}
	setlvl(this->_location_tree);
}

static std::string sanitizeURI(std::string uri) {
	std::string ret = uri;
	size_t		pos;

	while ((pos = ret.find("/./")) != std::string::npos)
		ret = ret.substr(0, pos) + ret.substr(pos + 2);
	while ((pos = ret.find("/../")) != std::string::npos) {
		size_t	pos2 = ret.substr(0, pos).find_last_of("/");
		if (pos2 == std::string::npos)
			ret = ret.substr(pos + 3);
		else
			ret = ret.substr(0, pos2) + ret.substr(pos + 3);
	}
	return ret;
}

static bool match_extension(std::string uri, std::string ext, size_t& pos) {

	if ((pos = uri.find_last_of(".")) == std::string::npos)
		return false;
	return uri.substr(pos) == ext;
}

bool	Request::is_cgi(LocationConfig *loc) {
	size_t	pos, npos;

	if (this->_request.state == ERROR
		|| this->_request.state == DONE
		|| !loc->add_cgi.size()
		|| !loc->cgi_path.size()
		|| !loc->cgi_allowed_methods.size())	return false;
	std::string uri = this->_request.first_line.uri;
	for (std::vector<std::string>::iterator it = loc->add_cgi.begin();it != loc->add_cgi.end(); ++it)
		if (match_extension(uri, *it, pos)) {
			std::string	temp_uri = uri.substr(pos);
			npos = temp_uri.find("/");
			if (npos != std::string::npos) {
				this->_cgi_info.second = temp_uri.substr(npos);
				temp_uri = temp_uri.substr(0, npos);
			}
			if (temp_uri == *it) { this->_cgi_info.first = *it; return true; }
			else	this->_cgi_info.second.clear();
		}
	return false;
}

void	Request::handle_location() {
	if (is_cgi(this->_c_location))	{this->_location_type = CGI; return ;}
	if (this->_c_location->return_url.first != STATUS_NONE)
		{setRequestState(this->_c_location->return_url.second, this->_c_location->return_url.first, DONE); this->_is_return = true;}
	if (std::find(this->_c_location->allowed_methods.begin(), this->_c_location->allowed_methods.end(), this->_request.first_line.method) == this->_c_location->allowed_methods.end())
		setRequestState(INV_MTH, NOT_IMPLEMENTED, ERROR);
	if (this->_location_type == CGI
		&& std::find(this->_c_location->cgi_allowed_methods.begin(),
			this->_c_location->cgi_allowed_methods.end(),
			this->_request.first_line.method) == this->_c_location->cgi_allowed_methods.end())
		setRequestState(INV_MTH, NOT_IMPLEMENTED, ERROR);
}

void	Request::setRequestState(std::string msg, e_status status, e_parser_state state) {
	if (this->_request.status != STATUS_NONE)
		return ;
	this->_request.error_message = msg;
	this->_request.status = status;
	this->_request.state = state;
}

bool	Request::is_file(std::string& path) {
	struct stat		st;

	if (stat(path.c_str(), &st) == -1)
		return false;
	if (S_ISREG(st.st_mode))
		return true;
	return false;
}

bool	Request::is_directory(std::string& path, int flag) {
	struct stat	st;

	if (stat(path.c_str(), &st) == -1)
		return false;
	if (S_ISDIR(st.st_mode) && access(path.c_str(), flag) == 0)
		return true;
	return false;
}

void	Request::handle_file() {
	if (access(this->_request.first_line.uri.c_str(), R_OK) == -1) {
		setRequestState(INV_LOC_FILE, FORBIDDEN, ERROR);
		this->_status = FORBIDDEN;
		this->_state = ERROR;
		return ;
	}
	else if (this->_location_type != CGI)	this->_location_type = STATIC;
}

void	Request::handle_directory(LocationConfig* loc) {
	std::string uri = this->_request.first_line.uri;

	if (access(this->_request.first_line.uri.c_str(), R_OK) == -1) {
		setRequestState(INV_LOC_DIR, FORBIDDEN, ERROR);
		this->_status = FORBIDDEN;
		this->_state = ERROR;
		return ;
	}
	if (loc->index.size() > 0) {
		uri += "/" + loc->index;
		if (is_file(uri)) {
			this->_request.first_line.uri = uri;
			this->_location_type = STATIC;
		}
		else if (loc->auto_index)
			this->_location_type = AUTOINDEX;
		else {
			this->_request.first_line.uri = uri;
			setRequestState(INV_LOC_FILE, NOT_FOUND, ERROR);
			this->_status = NOT_FOUND;
			this->_state = ERROR;
			return ;
		}
	}
	else if (!loc->auto_index) {
		setRequestState(INV_LOC_DIR, FORBIDDEN, ERROR);
		this->_status = FORBIDDEN;
		this->_state = ERROR;
	}
	else
		this->_location_type = AUTOINDEX;
}

void Request::handle_uri() {

	LocationConfig*	loc = this->_c_location;
	std::string r = loc->root + "/" + ((this->_location_type == CGI) ? (loc->cgi_path+"/") : "");

	this->_request.first_line.uri.replace(0, loc->path.size(), r);

	if (this->_location_type == CGI)	return ;

	if (this->_request.first_line.method == "GET" ) {
		if (is_file(this->_request.first_line.uri))
			handle_file();
		else if (is_directory(this->_request.first_line.uri, R_OK))
			handle_directory(loc);
		else
			setRequestState(NOT_FND, NOT_FOUND, ERROR);
	}
	else if (this->_request.first_line.method == "POST") {
		this->_request.first_line.uri = loc->root + "/" + loc->upload_store;
		if (is_directory(this->_request.first_line.uri, W_OK))
			this->_location_type = UPLOAD;
		else
			setRequestState(INV_LOC_DIR, FORBIDDEN, ERROR);
	}
	else if (this->_request.first_line.method == "DELETE") {
		if (access(this->_request.first_line.uri.c_str(), F_OK) == -1) {
			setRequestState(CANT_DELL, NOT_FOUND, ERROR);
			this->_status = NOT_FOUND;
			this->_state = ERROR;
			return ;
		}
		if (access(this->_request.first_line.uri.c_str(), W_OK) == -1) {
			setRequestState(CANT_DELL, FORBIDDEN, ERROR);
			this->_status = FORBIDDEN;
			this->_state = ERROR;
			return ;
		}
		this->_location_type = STATIC;
	}
}

static bool	is_valid_uri(std::string uri) {
	std::string	invalid = "<>\"{}|\\^[]`";

	for (size_t i = 0; i < invalid.size(); i++)
		if (uri.find(invalid[i]) != std::string::npos)
			return false;
	return true;
}

void	Request::parse_uri() {
	std::string			uri;

	uri = this->_request.first_line.uri;
	this->_request.first_line.uri = sanitizeURI(uri);
	if (is_valid_uri(uri) == false)
		{setRequestState(INV_URI, BAD_REQUEST, ERROR); return ;}
	extract_query_string();
	this->_c_location = ::search(this->_location_tree, this->_request.first_line.uri, ::cmp);
	if (this->_c_location == NULL) {
		setRequestState(LOC_NF, NOT_FOUND, ERROR);
		this->_status = NOT_FOUND;
		this->_state = ERROR;
		return ;
	}
	handle_location();
	handle_uri();
}

void Request::set_first_line() {
	if (this->_state != FIRST_LINE) return ;

	std::string		line;
	size_t			pos;

	if ((pos = this->_request_buffer.find("\r\n")) == std::string::npos)
		return ;
	this->_request.raw_first_line = this->_request_buffer.substr(0, pos);
	this->_request_buffer = this->_request_buffer.substr(pos + 2);
	this->_recv_bytes -= (pos + 2);
	parse_first_line();
	this->_state = HEADERS;
}

void	Request::set_headers() {
	if (this->_state != HEADERS) return ;

	size_t			pos;
	std::string		key;
	std::string		value;

	if ((pos = this->_request_buffer.find("\r\n\r\n")) == std::string::npos)
		return ;
	std::string headers = this->_request_buffer.substr(0, pos);
	this->_request_buffer = this->_request_buffer.substr(pos + 4);
	this->_recv_bytes -= (pos + 4);
	while ((pos = headers.find("\r\n")) != std::string::npos) {
		std::string line = headers.substr(0, pos);
		headers = headers.substr(pos + 2);
		if ((pos = line.find(":")) == std::string::npos)
			{setRequestState(NO_CL_HD, BAD_REQUEST, ERROR); continue ;}
		key = line.substr(0, pos);
		value = line.substr(pos + 2);
		this->_request.raw_headers[key] = value;
	}
	parse_headers();
	setLocation();
	parse_uri();
	this->_state = BODY;
}

void	Request::set_method() {
	if (this->_method != MTH_NONE)
		return ;
	std::string method = this->_request.first_line.method;
	if (method == "GET")
		this->_method = GET;
	else if (method == "POST")
		this->_method = POST;
	else if (method == "DELETE")
		this->_method = DELETE;
	else
		this->_method = NOT_IMP;
}

void	Request::set_body() {
	char	buffer[BUFFER_SIZE];
	int		ret;

	if (this->_state != BODY || this->_request.state == ERROR || this->_request.status != STATUS_NONE) {
		this->_status = this->_request.status;
		this->_state = this->_request.state;
		return ;
	}
	if (this->_method == GET || this->_method == DELETE)
		{this->_state = DONE; return ;}
	if (this->_request.headers.transfer_encoding == "chunked")
		handle_chunked();
	else if (this->_request.headers.content_length > this->_max_body_size) {
		setRequestState(BD_TOO_BIG, REQUEST_ENTITY_TOO_LARGE, ERROR);
		this->_state = ERROR;
		this->_status = REQUEST_ENTITY_TOO_LARGE;
		return ;
	}
	else if (this->_request.headers.content_length > 0)
		handle_content_length();
	else {
		setRequestState(LEN_REQ, LENGTH_REQUIRED, ERROR);
		this->_state = ERROR;
		this->_status = LENGTH_REQUIRED;
		return ;
	}
	write_to_file();
}

void Request::parse_first_line() {
	size_t			pos;
	std::string		uri;
	std::string		first_line;

	first_line = this->_request.raw_first_line;
	pos = first_line.find(" ");
	this->_request.first_line.method = first_line.substr(0, pos);
	first_line = first_line.substr(pos + 1);
	pos = first_line.find(" ");
	this->_request.first_line.uri = first_line.substr(0, pos);
	first_line = first_line.substr(pos + 1);
	pos = first_line.find("\r\n");
	this->_request.first_line.version = first_line.substr(0, pos);
	if (this->_request.first_line.method != "GET"
		&& this->_request.first_line.method != "POST"
		&& this->_request.first_line.method != "DELETE")
		setRequestState(INV_MTH, METHOD_NOT_ALLOWED, ERROR);
	if (this->_request.first_line.uri.empty())
		setRequestState(INV_URI, BAD_REQUEST, ERROR);
	if (this->_request.first_line.version != "HTTP/1.1")
		setRequestState(INV_VER, NOT_IMPLEMENTED, ERROR);
	set_method();
}

static std::string get_boundary(std::string content_type) {
	
	if (content_type.find("multipart/form-data") == std::string::npos)
		return "";
	size_t pos = content_type.find("boundary=");
	if (pos == std::string::npos)
		return "";
	std::string boundary = content_type.substr(pos + 9);
	boundary = "--" + boundary;
	return boundary;
}

void Request::parse_headers() {
	if (this->_state != HEADERS && this->_state != ERROR) return ;

	this->_request.headers.host = this->_request.raw_headers["Host"].substr(0, this->_request.raw_headers["Host"].find(":"));
	this->_request.headers.connection = this->_request.raw_headers["Connection"];
	size_t content_length = std::strtoll(this->_request.raw_headers["Content-Length"].c_str(), NULL, 10);
	this->_request.headers.content_length = content_length;
	this->_request.headers.transfer_encoding = this->_request.raw_headers["Transfer-Encoding"];
	this->_request.headers.content_type = this->_request.raw_headers["Content-Type"];
	this->_request.headers.date = this->_request.raw_headers["Date"];
	this->_request.headers.accept = this->_request.raw_headers["Accept"];
	this->_request.headers.location = this->_request.raw_headers["Location"];
	this->_request.headers.cookie = this->_request.raw_headers["Cookie"];
	this->_request.headers.set_cookie = this->_request.raw_headers["Set-Cookie"];
	this->_request.headers.user_agent = this->_request.raw_headers["User-Agent"];

	this->_request.boundary = get_boundary(this->_request.headers.content_type);
	if (this->_request.headers.host == "")
		setRequestState(MIS_HOST, BAD_REQUEST, ERROR);
}

static bool is_hex(std::string str) {
	for (size_t i = 0; i < str.size(); i++)
		if (!std::isxdigit(str[i]))
			return false;
	return true;
}

void Request::handle_content_length() {
	char	buffer[BUFFER_SIZE];
	int		ret;

	this->_request.raw_body.append(this->_request_buffer, 0, this->_recv_bytes);
	this->_total_body_size += this->_recv_bytes;
	/*std::cout << "Total body size : " << KRED << this->_total_body_size << KNRM << std::endl;
	std::cout << "Content length : " << KRED << this->_request.headers.content_length << KNRM << std::endl;*/
	// std::cout << "Total body size : " << KRED << this->_total_body_size << KNRM << std::endl;
	// std::cout << "Content length : " << KRED << this->_request.headers.content_length << KNRM << std::endl;
	this->_request_buffer.clear();
	if (this->_total_body_size == this->_request.headers.content_length) {
		while ((ret = recv(this->_fd, buffer, BUFFER_SIZE, MSG_PEEK | MSG_DONTWAIT)) != -1) {
			if (ret == 0) break ;
			setRequestState(LEN_NOT_MATCH, BAD_REQUEST, ERROR);
			this->_state = ERROR;
			this->_status = BAD_REQUEST;
			return ;
		}
		parse_body();
		return ;
	}
	if (this->_total_body_size > this->_request.headers.content_length) {
		setRequestState(BD_TOO_BIG, BAD_REQUEST, ERROR);
		this->_state = ERROR;
		this->_status = BAD_REQUEST;
		return ;
	}
}

static bool in_quotes(std::string str, size_t pos) {
	int	c = 0;

	for (size_t i = 0; i < pos; i++)
		if (str[i] == '"')
			c++;
	return (c % 2 == 1);
}

static bool is_filename(std::string section) {
	size_t	pos;
	bool	exists = false;

	std::string line = section.substr(0, section.find("\r\n"));
	while ((pos = line.find("filename=")) != std::string::npos) {
		if (in_quotes(line, pos)) {
			line = line.substr(pos + 9);
			continue ;
		}
		exists = true;
		break ;
	}
	if (exists) {
		section = section.substr(section.find("\r\n") + 2);
		line = section.substr(0, section.find("\r\n"));
		if (line.find("Content-Type:") != std::string::npos)
			return true;
	}
	return false;
}

static std::string get_field_value(std::string line, std::string field) {
	size_t	pos;

	while ((pos = line.find(field)) != std::string::npos) {
		if (in_quotes(line, pos)) {
			line = line.substr(pos + field.size() + 1);
			continue ;
		}
		break ;
	}
	std::string value = line.substr(pos + field.size() + 1);
	value = value.substr(0, value.find("\""));
	return value;
}

static	std::string get_random_file_name() {
	std::string		ret;
	std::string		chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	size_t			len = 10;

	srand(time(NULL));
	for (size_t i = 0; i < len; i++)
		ret += chars[rand() % chars.size()];
	return ret;
}

static	std::string get_section_filename(std::string data, std::string boundary, bool& is_file) {
	size_t		pos;
	std::string	line;

	is_file = false;
	pos = data.find(boundary);
	if (pos == std::string::npos)
		return "";
	data = data.substr(pos + boundary.size());
	if ((pos = data.find("\r\n")) == std::string::npos)
		return "";
	line = data.substr(pos + 2);
	if (is_filename(line))
		is_file = true;
	return get_field_value(line, "filename=");
}

void	Request::make_new_section() {
	bool	is_file = false;

	std::string filename = get_section_filename(this->_request.raw_body, this->_request.boundary, is_file);
	if (is_file) {
		filename = this->_c_location->root + "/" + this->_c_location->upload_store + "/" + filename;
		this->_file.open(filename.c_str(), std::ios::out | std::ios::binary);
		if (!this->_file.is_open()) {
			setRequestState(CANT_O_FILE, INTERNAL_SERVER_ERROR, ERROR);
			this->_state = ERROR;
			this->_status = INTERNAL_SERVER_ERROR;
			return ;
		}
	}
	t_post_raw post_raw = {filename, is_file, false, BOUNDRY, 0, ""};
	this->_post_raw.push_back(post_raw);
}

bool	Request::is_last_boundary() {
	std::string boundry = this->_request.boundary;

	if (this->_request.raw_body.substr(0, boundry.size() + 2) == (boundry + "--")) {
		this->_request.raw_body.clear();
		if (DEBUG) {
			std::cout << "Done with all sections" << std::endl;
			std::cout << "Sections size: " << this->_post_raw.size() << std::endl;
		}
		this->_state = DONE;
		return true;
	}
	return false;
}

void	Request::skip_boundary(t_post_raw& post_raw) {
	size_t			pos;
	std::string		boundry = this->_request.boundary;

	if ((pos = this->_request.raw_body.find(boundry)) == std::string::npos)
		{std::cout << "Boundry not found" << std::endl; return ;}
	this->_request.raw_body = this->_request.raw_body.substr(this->_request.raw_body.find("\r\n") + 2);
	post_raw.section = CONTENT_DIS;
}

void	Request::skip_content_dis(t_post_raw& post_raw) {
	size_t			pos;

	if ((pos = this->_request.raw_body.find("Content-Disposition:")) == std::string::npos)
		{std::cout << "Content-Disposition not found" << std::endl; return ;}
	this->_request.raw_body = this->_request.raw_body.substr(this->_request.raw_body.find("\r\n") + 2);
	post_raw.section = CONTENT_TYP;
}

void	Request::skip_content_typ(t_post_raw& post_raw) {
	size_t			pos;

	if ((pos = this->_request.raw_body.find("Content-Type:")) == std::string::npos) {
		this->_request.raw_body = this->_request.raw_body.substr(this->_request.raw_body.find("\r\n") + 2);
		post_raw.section = CONTENT;
		post_raw.is_file = false;
	}
	else {
		this->_request.raw_body = this->_request.raw_body.substr(this->_request.raw_body.find("\r\n") + 2);
		post_raw.section = EMPTY_LINE;
		post_raw.is_file = true;
	}
}

void	Request::skip_crlf(t_post_raw& post_raw) {
	size_t			pos;

	if ((pos = this->_request.raw_body.find("\r\n")) == std::string::npos)
		{std::cout << "CRLF not found" << std::endl; return ;}
	this->_request.raw_body = this->_request.raw_body.substr(pos + 2);
	post_raw.section = CONTENT;
}

void	Request::write_content(t_post_raw& post_raw) {
	size_t			pos;
	std::string		boundry = this->_request.boundary;

	if ((pos = this->_request.raw_body.find(boundry)) != std::string::npos) {
		this->_file.write(this->_request.raw_body.c_str(), pos - 2);
		if (this->_file.bad())
			{setRequestState(CANT_W_FILE, INTERNAL_SERVER_ERROR, ERROR); this->_state = ERROR; this->_status = INTERNAL_SERVER_ERROR; return ;}
		this->_file.close();
		post_raw.sec_size += (pos - 2);
		this->_request.raw_body = this->_request.raw_body.substr(pos);
		post_raw.finished = true;
		if (DEBUG)
			std::cout << "Done with file: " << KBLU << post_raw.filename << KNRM << " size: " << post_raw.sec_size << std::endl;
	}
	else {
		std::streampos	pos = this->_file.tellp();
		this->_file.write(this->_request.raw_body.c_str(), this->_request.raw_body.size());
		if (this->_file.bad())
			{setRequestState(CANT_W_FILE, INTERNAL_SERVER_ERROR, ERROR); this->_state = ERROR; this->_status = INTERNAL_SERVER_ERROR; return ;}
		std::streamsize bw = this->_file.tellp() - pos;
		post_raw.sec_size += bw;
		this->_request.raw_body = this->_request.raw_body.substr(bw);
	}
}

void	Request::store_content(t_post_raw& post_raw) {
	size_t			pos;
	std::string		boundry = this->_request.boundary;

	if ((pos = this->_request.raw_body.find(boundry)) != std::string::npos) {
		post_raw.data.append(this->_request.raw_body, 0, pos - 2);
		this->_request.raw_body = this->_request.raw_body.substr(pos);
		post_raw.finished = true;
		post_raw.sec_size += (pos - 2);
	}
	else {
		post_raw.data.append(this->_request.raw_body);
		post_raw.sec_size += this->_request.raw_body.size();
		this->_request.raw_body.clear();
	}
}

void	Request::handle_multipart() {
	size_t			pos;
	std::string 	boundry = this->_request.boundary;
	bool			is_file = false;

	while (this->_request.raw_body.size() > 0) {
		if (this->_post_raw.size() == 0)
			make_new_section();
		else if (this->_post_raw.back().finished) {
			if (is_last_boundary())
				return ;
			make_new_section();
		}
		if (this->_request.state == ERROR)
			return ;
		t_post_raw& post_raw = this->_post_raw.back();
		if (post_raw.section == BOUNDRY)
			skip_boundary(post_raw);
		if (post_raw.section == CONTENT_DIS)
			skip_content_dis(post_raw);
		if (post_raw.section == CONTENT_TYP)
			skip_content_typ(post_raw);
		if (post_raw.section == EMPTY_LINE)
			skip_crlf(post_raw);
		if (post_raw.section == CONTENT) {
			if (post_raw.is_file)
				write_content(post_raw);
			else
				store_content(post_raw);
		}
	}
}

void	Request::handle_raw_post() {
	char		buffer[100];
	int			ret;
	std::string	file_name;

	if (this->_post_raw.size() == 0) {
		file_name = get_random_file_name();
		this->_file.open(file_name.c_str(), std::ios::out | std::ios::binary);
		if (!this->_file.is_open())
			{setRequestState(CANT_W_FILE, INTERNAL_SERVER_ERROR, ERROR); this->_state = ERROR; this->_status = INTERNAL_SERVER_ERROR; return ;}
		t_post_raw post_raw = {file_name, true, false, BOUNDRY, 0, ""};
		this->_post_raw.push_back(post_raw);
	}
	std::streampos	pos = this->_file.tellp();
	this->_file.write(this->_request.raw_body.c_str(), this->_request.raw_body.size());
	if (this->_file.bad())
		{setRequestState(CANT_W_FILE, INTERNAL_SERVER_ERROR, ERROR); this->_state = ERROR; this->_status = INTERNAL_SERVER_ERROR; return ;}
	std::streamsize bytes_written = this->_file.tellp() - pos;
	this->_request.raw_body.substr(0, bytes_written);
	t_post_raw& post_raw = this->_post_raw.back();
	post_raw.sec_size += bytes_written;
}

void Request::write_to_file() {
	if (this->_location_type == CGI)
		return ;
	if (this->_request.boundary.size() > 0)
		handle_multipart();
	else
		handle_raw_post();
}


void Request::handle_chunked() {
	size_t			pos;
	std::string		tmp;

	while ((pos = this->_request_buffer.find("\r\n")) != std::string::npos || this->_chunk_size > 0) {
		if (this->_chunk_size > 0) {
			if (this->_chunk_size > this->_recv_bytes) {
				this->_request.raw_body.append(this->_request_buffer, 0, this->_recv_bytes);
				this->_chunk_size -= this->_recv_bytes;
				this->_total_body_size += this->_recv_bytes;
				this->_request_buffer.clear();
				break ;
			}
			else {
				this->_request.raw_body.append(this->_request_buffer.substr(0, this->_chunk_size), 0, this->_chunk_size);
				this->_request_buffer = this->_request_buffer.substr(this->_chunk_size);
				this->_total_body_size += this->_chunk_size;
				this->_recv_bytes -= this->_chunk_size;
				this->_chunk_size = 0;
				continue ;
			}
		}
		tmp = this->_request_buffer.substr(0, pos);
		this->_request_buffer = this->_request_buffer.substr(pos + 2);
		if (this->_chunk_size == 0 && is_hex(tmp)) {
			this->_chunk_size = std::strtoll(tmp.c_str(), NULL, 16);
			if (this->_chunk_size == 0) {parse_body(); return ;}
		}
		else
			{setRequestState(CANT_W_FILE, INTERNAL_SERVER_ERROR, ERROR); this->_state = ERROR; this->_status = INTERNAL_SERVER_ERROR; return ;}
	}
	if (this->_total_body_size > this->_max_body_size) {
		setRequestState(BD_TOO_BIG, REQUEST_ENTITY_TOO_LARGE, ERROR);
		this->_state = ERROR;
		this->_status = REQUEST_ENTITY_TOO_LARGE;
		return ;
	}
}

void Request::parse_body() {

	if (this->_request.body.size() == 0)
		this->_request.body = this->_request.raw_body;
	// if (this->_total_body_size == 0)
	// 	{this->_state = DONE; return ;}
	// if (this->_request.boundary.size() > 0)
	// 	parse_multipart();
	this->_state = DONE;
}

bool	Request::check_content_length(int ret) {
	char	buffer[10];
	int		b;

	if (ret < BUFFER_SIZE && this->_request.headers.content_length > 0) {
		while ((b = recv(this->_fd, buffer, 10, MSG_PEEK | MSG_DONTWAIT)) != -1) {
			if (b == 0 && this->_total_body_size + static_cast<size_t>(ret) < this->_request.headers.content_length) {
				// std::cout << "Ret : " << KRED << ret << KNRM << std::endl;
				// std::cout << "Content-Length " << KRED << this->_request.headers.content_length << KNRM << " does not match body size " << KRED << this->_total_body_size + static_cast<size_t>(ret) << KNRM << std::endl;
				setRequestState(LEN_NOT_MATCH, BAD_REQUEST, ERROR);
				this->_state = ERROR;
				this->_status = BAD_REQUEST;
				return false;
			}
			break ;
		}
		// std::cout << "length b : " << KRED << b << KNRM << std::endl;
		//std::cout << "length b : " << KRED << b << KNRM << std::endl;
	}
	return true;
}

void	Request::recvRequest() {
	char		buffer[BUFFER_SIZE];
	int			ret;

	if (this->_state == DONE || this->_state == ERROR) return ;
	ret = recv(this->_fd, buffer, BUFFER_SIZE, 0);
	this->_recv_bytes = ret;
	// std::cout << "Recv bytes : " << KRED << this->_recv_bytes << KNRM << std::endl;
	//std::cout << "Recv bytes : " << KRED << this->_recv_bytes << KNRM << std::endl;
	if (ret == -1 || ret == 0) {
		this->_status = INTERNAL_SERVER_ERROR;
		this->_state = ERROR;
		return ;
	}
	this->_request_buffer.append(buffer, ret);
	set_first_line();
	set_headers();
	if (check_content_length(ret) == false)
		return ;
	set_body();
}

std::string	urldecode(std::string str) {
	std::string ret;
	size_t		pos;

	while ((pos = str.find("%")) != std::string::npos) {
		ret += str.substr(0, pos);
		str = str.substr(pos + 1);
		ret += static_cast<char>(std::strtol(str.substr(0, 2).c_str(), NULL, 16));
		str = str.substr(2);
	}
	ret += str;
	while ((pos = ret.find("+")) != std::string::npos)
		ret.replace(pos, 1, " ");
	return ret;
}

void	Request::parse_query_string() {
	if (this->_query_string.size() == 0)
		return ;
	std::vector<std::pair<std::string, std::string> >::iterator it;
	for (it = this->_query_string.begin(); it != this->_query_string.end(); it++) {
		it->first = urldecode(it->first);
		it->second = urldecode(it->second);
	}
}

void	Request::extract_query_string() {
	size_t		pos;
	std::string	uri;

	if ((pos = this->_request.first_line.uri.find("#")) == std::string::npos)
		this->_request.first_line.uri = this->_request.first_line.uri.substr(0, pos);
	if ((pos = this->_request.first_line.uri.find("?")) == std::string::npos)
		return ;

	uri = this->_request.first_line.uri.substr(pos + 1);
	this->_request.first_line.uri = this->_request.first_line.uri.substr(0, pos);
	while ((pos = uri.find("&")) != std::string::npos) {
		std::string tmp = uri.substr(0, pos);
		size_t		pos2;
		if ((pos2 = tmp.find("=")) != std::string::npos)
			this->_query_string.push_back(std::make_pair(tmp.substr(0, pos2), tmp.substr(pos2 + 1)));
		else
			this->_query_string.push_back(std::make_pair(tmp, ""));
		uri = uri.substr(pos + 1);
	}
	if ((pos = uri.find("=")) != std::string::npos)
		this->_query_string.push_back(std::make_pair(uri.substr(0, pos), uri.substr(pos + 1)));
	else if (uri.size() > 0)
		this->_query_string.push_back(std::make_pair(uri, ""));
	parse_query_string();
}

void	Request::parse_multipart() {
	size_t			pos;
	std::string		boundary;
	std::string		section;
	std::string		line;

	pos = this->_request.raw_body.find(boundary);
	boundary = this->_request.boundary;
	while ((pos = this->_request.raw_body.find(boundary)) != std::string::npos) {
		section = this->_request.raw_body.substr(0, pos);
		this->_request.raw_body = this->_request.raw_body.substr(pos + boundary.size());
		if (section.empty()) continue ;
		section = section.substr(2);
		if ((pos = section.find("\r\n")) == std::string::npos) {
			setRequestState(INV_BD, BAD_REQUEST, ERROR);
			this->_state = ERROR;
			this->_status = BAD_REQUEST;
			return ;
		}
		if (is_filename(section))
			handle_post_file(section);
		else
			handle_post_fields(section);
		if (this->_state == ERROR)
			return ;
	}
}

void	Request::handle_post_file(std::string section) {

	size_t pos = section.find("\r\n");
	std::string line = section.substr(0, pos);

	std::string name = get_field_value(line, "name=");
	std::string filename = get_field_value(line, "filename=");
	section = section.substr(pos + 2);
	pos = section.find("Content-Type: ");
	std::string content_type = section.substr(pos + 14, section.find("\r\n") - pos - 14);
	content_type = content_type.substr(0, content_type.find("\r\n"));
	section = section.substr(section.find("\r\n\r\n") + 4);
	section = section.substr(0, section.find_last_of("\r\n") - 1);
	t_post_body post_body = {name, filename, content_type, section};
	this->_post_body.push_back(post_body);
}

void	Request::handle_post_fields(std::string section) {

	size_t pos = section.find("\r\n");
	std::string line = section.substr(0, pos);
	std::string name = get_field_value(line, "name=");
	section = section.substr(pos + 4);
	section = section.substr(0, section.find_last_of("\r\n") - 1);
	t_post_body post_body = {name, "", "", section};
	this->_post_body.push_back(post_body);
}

e_status	Request::getStatus( void ) {
	return this->_status;
}

e_parser_state	Request::getState( void ) {
	return	this->_state;
}

t_first_line	Request::get_first_line() { return this->_request.first_line; }
t_headers		Request::get_headers() { return this->_request.headers; }
e_location_type	Request::get_location_type() { return this->_location_type; }
void			Request::set_fd(int sock_fd) { this->_fd = sock_fd; }
void			Request::setStatus(e_status status) { this->_status = status; }
void			Request::set_servers(std::vector<ServerConfig*>& servers) { this->_servers = servers; }
