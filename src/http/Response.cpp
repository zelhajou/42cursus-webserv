#include "Response.hpp"


Response::Response():_new_session(false),  _file_type("NONE"), _has_cookies(false), status(FIRST_LINE), _has_body(true) {
	this->_sent.push_back(0);
	this->_sent.push_back(0);
	this->_recv.push_back(0); this->_recv.push_back(0);
	this->_recv.push_back(0); this->_recv.push_back(0);
}

Response::Response(const Response &R) { *this = R; }
Response	&Response::operator = (const Response &R) { (void)R; return *this; }
Response::~Response() {}

size_t	Response::get_file_size() {
	std::streampos	fpos = this->_file.tellg();
	this->_file.seekg(0, std::ios::end);
	size_t	size = this->_file.tellg()-fpos;
	this->_file.seekg(0, std::ios::beg);
	return (size);
}

std::string	Response::http_code_msg(e_status code)
{
	if (this->_request->_is_return)		
		return this->_request->_c_location->return_url.second;
	switch (code) {
		case OK:				return "OK";
		case BAD_REQUEST:			return "Bad Request";
		case NOT_FOUND:			return "Not Found";
		case FORBIDDEN:			return "Forbidden";
		case INTERNAL_SERVER_ERROR:		return "Internal Server Error";
		case NOT_IMPLEMENTED:		return "Not Implemented";
		case REDIRECT:			return "Redirect";
		case NOT_MODIFIED:			return "Not Modified";
		case TOO_MANY_REQUESTS:		return "Too Many Requests";
		case REQUEST_ENTITY_TOO_LARGE:	return "Request Entity Too Large";
		case REQUEST_HEADER_FIELDS_TOO_LARGE:	return "Request Header Fields Too Large";
		case HTTP_VERSION_NOT_SUPPORTED:	return "Http Version NOT Supported";
		case URI_TOO_LONG:			return "Url Too Long";
		case LENGTH_REQUIRED:		return "Length Required";
		case REQUEST_TIMEOUT:		return "Request Timeout";
		case NOT_ACCEPTABLE:		return "Not Acceptable";
		case METHOD_NOT_ALLOWED:		return "Method Not Allowed";
		default:				return "";
	}
}



std::string	Response::generate_status_file(e_status status_code, ServerConfig *server, std::string addon) {
	std::map<int, std::string>::iterator	it = server->error_pages.find(status_code);
	if (it != server->error_pages.end() && access(it->second.c_str(), R_OK) == 0) return it->second;
	if (!addon.size())	addon = this->http_code_msg(status_code);
	std::string	status_file = conc_urls(CONFIG_PATH, "html_default_error_files/") + std::to_string(status_code) + clean_up_stuff(addon, "[\\]^`:;<>=?/ ", "_____________") + ".html";
	if (!access(status_file.c_str(), R_OK))	return status_file;
	std::fstream	_file(status_file, std::ios::out);
	if (!_file.is_open())	return "";
	std::string mdn_link = "https://developer.mozilla.org/en-US/docs/Web/HTTP/Status/" + std::to_string(status_code);
	_file << "<html><head><title>"<<std::to_string(status_code)<<"</title><style>\
			* {margin:0px;padding:0px;box-sizing:border-box;}html{background-color:#000;}\
			body {height:100vh;width:100%;display:flex;align-items:center;row-gap:1em;\
				justify-content:center;flex-direction:column;font-family:monospace;}\
			h1 {color:#fff;font-size:15em;font-style:italic;text-shadow:5px 5px #7a7a7a;}\
			h2 {color:#7a7a7a;text-transform:uppercase;font-size:3em;}a{font-size:20px;}\
		</style></head><body><h1>" << std::to_string(status_code) << "</h1><h2>" << addon << "</h2>\
		<a target='_blank' href=\"" << mdn_link << "\">developer.mozilla.org</a></body></html>";
	_file.close();
	this->_response_status = status_code;
	return	status_file;
}

static	std::string	replace_characters(std::string input, std::string from, std::string to) {
	size_t	i;
	while ((i = input.find(from)) != std::string::npos)
		input.replace(i, from.size(), to);
	return	input;
}

static	size_t		get_dir_size(std::string path) {
	struct	stat	output;
	if (stat(path.c_str(), &output) == -1)	return 0;
	return	output.st_size;
}

e_status	print_Cstatus(e_status st) {
	int	status = (int)st;
	std::cout << KBGR;
	if (status == 200)		std::cout << KGRN;
	else if (status >= 600)	std::cout << KWHT;
	else if (status >= 500)	std::cout << KYEL;
	else if (status >= 400)	std::cout << KRED;
	else if (status >= 300)	std::cout << KCYN;
	std::cout << " " << st << " ";
	return	st;
}

static	std::string	generate_auto_index(std::string uri, ServerConfig *server) {
	(void)server;
	std::string target =  conc_urls(CONFIG_PATH, "html_generated_files/") + replace_characters(uri, "/", "#")+"S-"+std::to_string(get_dir_size(uri))+"Bytes"+".html";
	struct	stat	demo;
	if (stat(target.c_str(), &demo) != -1)	return target;
	std::fstream	output(target, std::ios::out);
	if (!output)	return	"";
	output << "<html><head><title> | Directory Listing</title>\
		<style>*{margin:0px;padding:0px;box-sizing:border-box;}html{background-color:#000;}body{\
		height:100vh;width:100%;display:flex;align-items:center;justify-content:center;flex-direction:column;font-family:monospace;color:#f7f7f7;}\
		#container{height:100%;width:90%;display:flex;border:1px solid #404040;flex-direction:column;justify-content:space-around;align-items:start;\
		max-height:90%;overflow:auto;scroll-behavior:smooth;padding:4em;}h1{color:#c8c8c8;font-size:1.7em;}\
		#container > div{height:90%;width:100%;display:flex;flex-direction:column;padding:2em 5em;align-items:start;background-color:#333;}\
		a{font-size:1.3em;height:2em;width:100%;display:flex;flex-direction:row;justify-content:space-between;align-items:center;\
		transition-duration:.3s;font-family:monospace;text-decoration:none;color:inherit;padding:0em 10px;}p{font-size:0.9em;\
		color:inherit;}hr {background-color:#4a4a4a;width:99%;height:1px;border:0px;align-self:center;}\
		a:hover{background-color:#f7f7f7;color:#333;}</style>\
		<script>function c_redir(loc){if (window.location.href.endsWith(\"#\")) window.location.href = window.location.href.slice(0, -1);\
		if (!window.location.href.endsWith(\"/\")) loc = \"/\" + loc; window.location.href += loc;}</script>\
		</head><body><div id=\"container\">\
		<h1>" << uri << " :</h1><div>";
	DIR			*dir = opendir(uri.c_str());
	struct	dirent		*direct;
	struct	stat		file_status;
	std::string		new_uri;
	while ((direct = readdir(dir))) {
		new_uri = conc_urls(uri, direct->d_name);
		output << "<a href=\"";
		/*if (!std::strcmp(direct->d_name, ".") || !std::strcmp(direct->d_name, ".."))
			output << direct->d_name<< "\">" << direct->d_name;
		else	*/output << "javascript:;\" onclick=\"c_redir('" << direct->d_name << "')\">" << direct->d_name;
		std::memset(&file_status, 0, sizeof(file_status));
		if (stat(new_uri.c_str(), &file_status) == -1) {
			output << "<p style=\"color:red;\">size_retrieval_failed</p></a><hr/>";
			continue;
		}
		output <<"<p>"<< std::to_string(file_status.st_size)<<" B</p></a><hr/>";
	}
	output <<	"</div></div></body></html>\n";
	output.close();
	closedir(dir);
	return	target;
}

e_parser_state	Response::get_status() { return this->status; }

void	Response::file_to_disk(int upload_buffer_size) {
	try {
		if (this->_raw_upload) {
			if (!this->_recv[1]) {
				this->_recv[0] = 0;
				this->_recv[1] = this->_request->_request.raw_body.size();
				this->_upload_target = conc_urls(this->_upload_path, _generate_random_string(this->_upload_path, 15));
				this->_upload_stream.open(this->_upload_target, std::ios::out);
				if (!this->_upload_stream) throw std::runtime_error("cant open upload stream");
			}
			if (this->_recv[0] + upload_buffer_size > this->_recv[1]) upload_buffer_size = this->_recv[1] - this->_recv[0];
			this->_upload_stream << this->_request->_request.raw_body.substr(this->_recv[0], upload_buffer_size);
			this->_recv[0] += upload_buffer_size;
		} else {
			if (!this->_recv[3]) {
				this->_recv[1] = 0; this->_recv[2] = 0;
				this->_recv[3] = this->_request->_post_body.size();
			}
			if (!this->_recv[1]) {
				this->_recv[0] = 0;
				this->_form_field = &this->_request->_post_body[ this->_recv[2] ];
				this->_recv[1] = this->_form_field->data.size();
				if (this->_recv[1] == 0) { this->_recv[2] += 1; return ; }
				if (!this->_form_field->filename.size()) this->_form_field->filename = _generate_random_string(this->_upload_path, 15);
				this->_upload_target = conc_urls(this->_upload_path, this->_form_field->filename);
				if (this->_upload_stream.is_open()) this->_upload_stream.close();
				this->_upload_stream.open(this->_upload_target, std::ios::out);
				if (!this->_upload_stream) throw std::runtime_error("cant open upload stream");
			}
			if (this->_recv[0] + upload_buffer_size > this->_recv[1]) upload_buffer_size = this->_recv[1] - this->_recv[0];
			this->_upload_stream << this->_form_field->data.substr(this->_recv[0], upload_buffer_size);
			this->_recv[0] += upload_buffer_size;
			if (this->_recv[0] >= this->_recv[1]) {
				this->_recv[0] = 0; this->_recv[1] = 0; this->_recv[2] += 1;
			}
		}
	} catch (std::exception &l) {(void)l;
		this->_post_status = false;
		this->_recv[0] = this->_recv[1];
		return ;
	}
}

void	Response::_initiate_response(int client, Sockets &sock, ServerConfig *server) {
	if (this->_request->getStatus() == OK && !this->_request->_is_return) {
		if (this->_request->_location_type == CGI) {/*hold_it*/}
		else if (this->_request->_location_type == AUTOINDEX) {
			struct	stat	output;
			std::string	uri = this->_request->get_first_line().uri;
			std::memset(&output, 0, sizeof(output));
			if (stat(uri.c_str(), &output) == -1 || !S_ISDIR(output.st_mode)
				|| !(this->target_file = generate_auto_index(uri, server)).size()) {
				this->_request->setStatus(FORBIDDEN);
				this->_has_body = false;
			}
		}
		else if (this->_request->get_first_line().method == "GET") this->target_file = this->_request->get_first_line().uri;
		else if (this->_request->get_first_line().method == "POST") {
			this->status = UPLOADING;
			this->_post_status = true;
			this->_recv[0] = 0; this->_recv[1] = 0;
			this->_recv[2] = 0; this->_recv[3] = 0;
			this->_upload_path = this->_request->get_first_line().uri;
			fcntl(client, F_SETFL, O_NONBLOCK);
			if (this->_request->get_headers().content_type.find("multipart/form-data") != std::string::npos)
				this->_raw_upload = false;
			else	this->_raw_upload = true;
			this->file_to_disk(UPLOAD_BUFFER_SIZE);
			return ;
		}
		else if (this->_request->get_first_line().method == "DELETE")
			this->target_file = this->generate_status_file((std::remove(this->_request->get_first_line().uri.c_str()))
				? INTERNAL_SERVER_ERROR
				: this->_request->getStatus(), server, "");
	}
	else	this->target_file = this->generate_status_file(this->_request->getStatus(), server, "");
	this->_begin_response(sock, server, 0);
}

void	Response::_begin_response(Sockets &sock, ServerConfig *server, int st) {
	if (st)	this->target_file = this->generate_status_file(INTERNAL_SERVER_ERROR, server, "");
	if (this->_has_body) {
		this->_file.open(this->target_file, std::ios::in|std::ios::binary);
		if (!this->_file) {
			e_status	tempS = this->_request->getStatus();
			this->_request->setStatus(FORBIDDEN);
			if (tempS == OK) {
				this->_initiate_response(0, sock, server);
				return ;
			}
			else	this->_has_body = false;
		}
		else {
			this->_file_size = this->get_file_size();
			size_t	ppos = this->target_file.rfind(".");
			if (ppos == this->target_file.npos)	ppos = 0;
			if (this->_file_type == "NONE")
				this->_file_type = sock.get_mime_type(this->target_file.substr(ppos));
		}
	}
	this->_connection_type = (this->_request->get_headers().connection == "keep-alive") ? "keep-alive" : "close";
}

size_t	Response::form_headers(ServerConfig *server) {
	e_status	req_scode = this->_response_status;
	if (this->header.size())	this->header.clear();
	this->header = "HTTP/1.1 " + std::to_string(req_scode) + " " + this->http_code_msg(req_scode) + CRLF;
	this->header.append("Server: " + server->server_name + CRLF"Connection: " + this->_connection_type + CRLF);
	if (this->_request->_is_return && req_scode >= 300 && req_scode < 400)
		this->header.append("Location: " + this->_request->_c_location->return_url.second + CRLF);
	else if (this->_has_redir)	this->header.append("Location: " + this->_cgi_redir + CRLF);
	if (this->_has_cookies)	this->header.append("set-cookie: " + this->_cgi_cookie + " ;" + (this->_new_session ? "" : CRLF));
	if (this->_new_session)	this->header.append((this->_has_cookies ? std::string("") : std::string("set-cookie: ")) + "session="+ this->_session_id + "; " + CRLF);
	if (this->_has_body) {
		this->header.append("Content-type: " + this->_file_type + CRLF);
		this->header.append("Content-Length: " + std::to_string(this->_file_size) + CRLF);
	}
	this->header.append(CRLF);
	return this->header.size();
}

void	Response::sendResponse(int sock_fd, Sockets &sock, ServerConfig *server) {
	int		sent_res;

	if (this->status == UPLOADING) {
		this->file_to_disk(UPLOAD_BUFFER_SIZE);
		if ((this->_raw_upload && this->_recv[0] >= this->_recv[1])
			|| (!this->_raw_upload && this->_recv[2] >= this->_recv[3])) {
			if (this->_upload_stream.is_open()) this->_upload_stream.close();
			this->target_file = this->_post_status ?
				this->generate_status_file(this->_request->getStatus(), server, "") :
				this->generate_status_file(INTERNAL_SERVER_ERROR, server, "UPLOAD FAILURE");
			this->_begin_response(sock, server, 0);
			this->status = FIRST_LINE;
		}
		else if (DEBUG) std::cout << "\t  [" << server->server_name << " upload: " << this->_recv[0] << " of " << this->_recv[1]
				<< ", file " << (this->_raw_upload ? "" : (std::to_string(this->_recv[2]+1) + " of " + std::to_string(this->_recv[3]))) << "]" << std::endl;
		return ;
	}
	if (this->status == FIRST_LINE) {
		this->_sent[1] = this->form_headers(server);
		this->status = HEADERS;
		return ;
	}
	if (this->status == HEADERS) {
		sent_res = send(sock_fd, this->header.c_str(), this->header.size(), 0);
		if (sent_res < 0)	this->_sent[0] = this->_sent[1];
		else		this->_sent[0] += sent_res;

		if (this->_sent[0] >= this->_sent[1]) {
			this->status = DONE;
			if (this->_has_body) {
				this->_sent[1] = 0;
				this->status = BODY;
			}
		}
		else	this->header = this->header.substr(sent_res);
	}
	else if (this->status == BODY) {
		char	buffer[FILE_READ_BUFFER_SIZE];

		std::memset(buffer, 0, sizeof(buffer));
		if (!this->_sent[1]) {
			this->_sent[1] = this->_file_size;
			this->_sent[0] = 0;
		}
		this->_file.read(buffer, FILE_READ_BUFFER_SIZE);
		sent_res = this->_file.gcount();
		for (int bytes_sent=0, temp_perc=0; bytes_sent < sent_res;) {
			temp_perc = send(sock_fd, buffer, sent_res, 0);
			if (temp_perc < 0) {
				this->_sent[0] = this->_sent[1];
				break ;
			}
			bytes_sent += temp_perc;
		}
		this->_sent[0] += sent_res;
		if (this->_sent[0] >= this->_sent[1])	this->status = DONE;
	}
	if (this->status == DONE) {
		this->_file.close();
		if (this->_request->_location_type == CGI) std::remove(target_file.c_str());
		if (DEBUG) {
			size_t	m_size = this->_request->get_first_line().method.size();
			size_t	u_size = this->_request->get_first_line().uri.size();
			std::cout << "[ " << this->_request->get_first_line().method << std::setw(7-m_size) << " ]" << KNRM
			<< " " << KUND << this->_request->get_first_line().uri << " " << std::setw(80-u_size) << KNRM
			<< this->http_code_msg(print_Cstatus(this->_response_status)) << " " << KNRM << " : "
			<< KUND << "\x1b[3m" << this->_sent[1] << " B" << KNRM << std::endl;
		}
	}
}

