#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <dirent.h>

//	request paths
#define	WWW_PATH		"/cgi_scripts/cpp_session_app"
#define	WWW_UPLOAD_PATH	WWW_PATH"/uploads"
#define	WWW_FONTS_PATH	WWW_PATH"/fonts"
#define	APP_PATH		"/cpp_session_app/cgi_.cpp"
//	initial paths
#define	ABS_PATH		"/Users/beddinao/cursus-0/webserv/mainWebserver/www/cgi_scripts/cpp_session_app"
#define	DATA_FILE		ABS_PATH"/database"
#define	UPLOAD_PATH	ABS_PATH"/uploads"
#define	COMMON_CSS	"<style>* {margin:0px;padding:0px;box-sizing:border-box;}html{background-color:#fff;}\
			body {height:100vh;width:100%;display:flex;align-items:center;row-gap:1em;\
			justify-content:center;flex-direction:column;font-family:monospace;}\
			h1 {color:#000;font-size:10em;font-style:italic;text-shadow:5px 5px #7a7a7a;}\
			h2 {color:#7a7a7a;text-transform:uppercase;font-size:3em;}a{font-size:20px;}</style>"
#define	ERROR_PAGE	"<body><h1>LOGIN_APP : INVALID REQUEST</h1></body>"
#define	INVALID_MTH	"<body><h1>LOGIN_APP : INVALID METHOD</h1></body>"
#define	ACCESS_DENIED_PAGE	"<body><h1>LOGIN_APP : ACCESS DENIED</h1></body>"
#define	_USER_ST_DEL_	"_ST_USER_"
#define	_USER_EN_DEL_	"_EN_USER_"
#define	UNIQ_FI		"_not__obv_ious_"
#define	SESSION_ID_LENGTH	15


std::string	generate_signup_html(const char *uri) {
		std::string	html = "<html><head><title> | SIGN UP</title><style>*{box-sizing: border-box;  padding: 0px;  margin: 0px;}\
		@font-face{font-family:\"Mega\"; src: url(\""WWW_FONTS_PATH"/LexendMega-Regular.ttf\") format(\"truetype\") ;}\
		body {  height: 100vh;  witdh: 100%;  background: #d2d2d2;  color: #282828;  font-family: Mega;}\
		body, div, form {  display: flex;  align-items: center;  justify-content: center;  flex-direction: column;}\
		#container {  border-left: 1px solid #282828;  height: 50%;  width: 40%;  justify-content: start;}\
		#container h1 {  font-size: 1.4em;  width: 90%;}#container form {  height: 100%;  width: 90%;  row-gap: 5em;  align-items: start;}\
		input{  border: 2px solid #282828;  background: transparent;  width: 100%;  height: 3em;\
		color: #282828;  font-family: monospace;  padding-left: 1em;  font-size: 25px;  transition-duration: .3s;}\
		input[type=submit] {  padding : 0px 0px;  width: 30%;  height: 2em;  border-radius: 5px;  cursor: pointer;}\
		input[type=submit]:hover {  color: #282828;  background: #d2d2d2;}input:focus {  border: 2px solid white;  outline: none;}\
		a, a:hover, a:visited{  color: #282828;}p {  font-size: 20px;  width: 90%;}</style>\
		</head><body><div id=\"container\"><h1>this session is not yet registered you must sign up to continue</h1>\
		<form method=\"POST\" action=\""APP_PATH"\" enctype=\"application/x-www-form-urlencoded\">\
		<input type=\"text\" name=\"user_name\" placeholder=\"user name\" required>\
		<input type=\"text\" name=\"password\" placeholder=\"password\" required>\
		<input type=\"submit\" value=\"SUBMIT\" /></form><p>already have an<br>account?\
		<a href=\""APP_PATH"\" >refresh</a></p></div>  </body></html>";
		return	html;
}

std::string	generate_home_html(const char *uri, std::string user_name) {
	std::string	html = "<!DOCTYPE html><html><head><title> | HOME</title><style>*{  box-sizing: border-box;  padding: 0px;  margin: 0px;}\
			        @font-face{font-family:\"Kodchasan\"; src: url(\""WWW_FONTS_PATH"/Kodchasan-MediumItalic.ttf\") format(\"truetype\") ;}\
			        @font-face{font-family:\"Mega\"; src: url(\""WWW_FONTS_PATH"/LexendMega-Regular.ttf\") format(\"truetype\") ;}\
				body {  height: 100vh;  witdh: 100%;  background: #d2d2d2;  color: #282828;  font-family: Mega;}\
				body, div, form {  display: flex;  align-items: center;  justify-content: center;  flex-direction: column;}\
				body {  flex-direction: row;  justify-content: space-around;}body > div {  border: 1px solid #282828;\
				height: 90%;  width: 46%;  justify-content: start;  align-items: start;  row-gap: 2em;  padding: 2em;}\
				#container_1 {  border: none;}h1 {  font-size: 1.4em;  max-width: 30em;  line-height: 1.5em;}\
				h2 {  font-size: 1.2em;  text-decoration: underline;  cursor: pointer; font-family: Kodchasan}\
				span {  text-decoration: underline;}.off {  display: none;}\
				#container_2, #container_3 {  flex-direction: column;  align-items: center;}\
				#container_2 form {  height: 90%;  width: 90%;  row-gap: 2em;}\
				input{  border: 2px solid #282828;  background: transparent;  width: 90%;  height: 3em;\
				color: #282828;  font-family: monospace;  padding-left: 1em;  font-size: 20px;\
				transition-duration: .3s;}input[type=submit] {  padding : 0px 0px;  width: 30%;  height: 2em;\
				border-radius: 5px;  cursor: pointer;}input[type=submit]:hover {  color: #282828;  background: #A5A5A5;}\
				input:focus {  border: 2px solid white;  outline: none;}#content {  height: 90%;  width: 100%;  padding: 2em;\
				justify-content: start;  row-gap: 2em;  overflow: auto;}a, a:hover, a:visited {  color: white;  font-size: 1.5em;}\
				</style><script>function a_one() {document.getElementById(\"container_2\").classList.remove(\"off\");\
				document.getElementById(\"container_3\").classList.add(\"off\");} function a_two() {\
				document.getElementById(\"container_3\").classList.remove(\"off\");document.getElementById(\"container_2\").classList.add(\"off\");}\
				</script></head><body><div id=\"container_1\"><h1>welcome <span>"+user_name+"</span><br><br>this application is a small demonstration\
				<br> of how web_servers with the help of browsers manages sessions.<br><br>your login credentials will be asked only\
				once you start a new session (change your browser/erase your cookies) and every time you visit this site we \
				will remember you which mean you don\'t need to login again.<br><br>here is some things you can do in here:\
				</h1><br><br><h2 id=\"upload\" onclick=\"a_one()\">upload some files</h2><h2 id=\"see_fi\" onclick=\"a_two()\">see files you uploaded</h2></div>\
				<div id=\"container_2\" ><h1>UPLOAD</h1><form method=\"POST\" action=\""APP_PATH"\"\
				enctype=\"multipart/form-data\" ><input type=\"file\" name=\"file\" placeholder=\"your file\" required>\
				<input type=\"submit\" value=\"SUBMIT\" /></form></div><div id=\"container_3\" class=\"off\" ><h1>YOUR FILES</h1>\
				<div id=\"content\">";
	//////////////////
	DIR		*dir = opendir(UPLOAD_PATH);
	struct	dirent	*direct;
	int		np;
	while ((direct = readdir(dir))) {
		std::string	file_name(direct->d_name);
		if ((np = file_name.find(user_name + UNIQ_FI)) != std::string::npos) {
			file_name = file_name.substr(np + user_name.size() + std::strlen(UNIQ_FI));
			html.append("<a href=\""WWW_UPLOAD_PATH"/" + std::string(direct->d_name) + "\" >" + file_name + "</a>");
		}
	}
	closedir(dir);
	//////////////////
	html.append("</div></div></body></html>");
	return	html;
}

std::string	clean_up_stuff(std::string input, std::string garbage, std::string target) {
	if (input.empty()||garbage.empty()||garbage.size() != target.size())	return "";
	int				pos;
	for (int i=0; i < garbage.size(); i++)
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

static	std::string	_generate_random_string(std::string seed) {
	std::string	output;
	for (int i=0;i<SESSION_ID_LENGTH;i++) {
		std::string    r(1, static_cast<char>(std::rand() % (122 - 48) + 48 ));
		output.append( r );
	}
	return	clean_up_stuff(output, "[\\]^`:;<>=?/ ", "_____________");
}

std::string	get_user_name() {
	int		pos, r;
	bool		_found(false);
	std::string	buffer, session_id;
	//
	const	char *C_K = std::getenv("HTTP_COOKIE");
	if (!C_K)		return	"";
	std::string	COOKIE(const_cast<char*>(C_K));
	if ((pos = COOKIE.find("login_app_session_id=")) == std::string::npos) return "";
	session_id = COOKIE.substr(pos + 21);
	session_id = session_id.substr(0, SESSION_ID_LENGTH);
	//
	std::fstream	_file(DATA_FILE, std::ios::in);
	std::cout << "FILE_is_NOT_OPEN" << std::endl;
	if (!_file.is_open())	return	"";
	for (;std::getline(_file, buffer);) {
		std::cout << "FILE_iS_oPEN" << std::endl;
		if (_found) {
			_file.close();
			return	buffer.substr(11, buffer.find("\n"));
		}
		if ((pos = buffer.find("--session:")) != std::string::npos
			&& (pos = buffer.find(session_id)) != std::string::npos) {
			_found = true;
		}
	}
	return	"";
}

std::string	handle_file_submission(const char *uri, std::string user_name, std::string input) {
	int	pos = input.find("filename="), np;
	try {
		if (pos == std::string::npos) {pos = input.find("name="); input = input.substr(pos + 5);}
		else	input = input.substr(pos + 9);
		if (pos != std::string::npos) {
			if (input[0] == '\"' || input[0] == '\'') {
				input = input.substr(1);
				pos = input.find("\"");
				if (pos == std::string::npos)
					pos = input.find("\'");
			}
			else	pos = input.find(" ");
			if (pos == std::string::npos)
				return	'\n' + generate_home_html(uri, user_name) + '\n';
			std::string	file_name = input.substr(0, pos);
			std::fstream	_file(UPLOAD_PATH"/" + user_name + UNIQ_FI +  clean_up_stuff(file_name, "[\\]^`:;<>=?/ ", "_____________"), std::ios::out);
			for (int i=0; i<3; i++) {
				int p = input.find("\n");
				input.erase(0, p + 1);
			}
			for (int i=0; i<2; i++) {
				int p = input.rfind("\n");
				input.erase(p - 1);
			}
			if (_file.is_open()) {
				_file << input;
				_file.close();
			}
		}
	}
	catch (std::exception &l) {/*nothing happened*/}
	return	'\n' + generate_home_html(uri, user_name) + '\n';
}

std::string	handle_signup(const char *uri, std::string query_string) {
	int			pos, p;
	std::vector<std::string>	query;
	pos = query_string.find("&");
	query.push_back(query_string.substr(0, pos));
	query.push_back(query_string.substr(pos + 1));
	for (int i=0; i < 2; i++) {
		p = query[i].find("=");
		query[i] = query[i].substr(p + 1);
	}
	//
	std::string		session_id = _generate_random_string(query_string);
	std::cout	<< "set-cookie: login_app_session_id=" << session_id << '\n';
	//
	std::fstream	_file(DATA_FILE, std::ios::in|std::ios::out);
	if (_file.is_open()) {
		_file.seekp(0, std::ios::end);
		_file << _USER_ST_DEL_ << session_id << _USER_ST_DEL_ << '\n';
		_file << "--session:" << session_id << '\n';
		_file << "--username:" << clean_up_stuff(query[0], "[\\]^`:;<>=?/ ", "_____________") << '\n';
		_file << "--password:" << query[1] << '\n';
		_file << _USER_EN_DEL_ << session_id << _USER_EN_DEL_ << '\n';
	}
	//
	return	'\n' + generate_home_html(uri, query[0]) + '\n';
}

int	main() {
	const	char*	R = std::getenv("REQUEST_METHOD");
	const	char*	URI = std::getenv("REQUEST_URI");
	if ( !R || !URI ) {
		std::cout << "Content-type: text/html\n\n<html><head>"COMMON_CSS"</head>"ERROR_PAGE"</html>\n";
		return 1; }
	std::srand(std::time(NULL));
	std::string	user_name = get_user_name();
	bool		is_registered_user = (user_name.empty() ? false : true);
	///
	std::cout << "Content-type: text/html\n";
	if (!std::strcmp(R, "POST")) {
		
		const	char	*C_L = std::getenv("CONTENT_LENGTH");
		const	char	*C_T = std::getenv("CONTENT_TYPE");
		if (!C_L || !C_T) {
			std::cout << "\n<html><head>"COMMON_CSS"</head>"ERROR_PAGE"</html>\n";
			return 1; }
		size_t		CONTENT_LENGTH = std::atoi(C_L);
		char		buffer[101];
		int		r(0);
		std::string	input;
		while (true) {
			std::memset(buffer, 0, sizeof(buffer));
			r = read(0, buffer, 100);
			if (r <= 0)	break;
			input.append(buffer, r);
		}
		std::string	CONTENT_TYPE(const_cast<char*>(C_T));
		if (CONTENT_TYPE.find("multipart/form-data") != std::string::npos
			&& is_registered_user)
			std::cout << handle_file_submission(URI, user_name, input);
		else if (CONTENT_TYPE.find("application/x-www-form-urlencoded") != std::string::npos) {
			if (is_registered_user)
				std::cout << '\n' << generate_home_html(URI, user_name);
			else	std::cout << handle_signup(URI, input);
		}
		else	std::cout << "\n<html><head>"COMMON_CSS"</head>"INVALID_MTH"</html>\n";
	}
	else if (!std::strcmp(R, "GET")) {
		const	char	*QUERY_STRING = std::getenv("QUERY_STRING");
		if (QUERY_STRING) {
			int		pos(0);
			std::string	query(const_cast<char*>(QUERY_STRING));
			if ((pos = query.find("login")) != std::string::npos && pos == 0) {
				std::cout << generate_signup_html(URI) << '\n';
				return	0; }
		}
		if (is_registered_user)	std::cout << '\n' << generate_home_html(URI, user_name) << '\n';
		else			std::cout << '\n' << generate_signup_html(URI) << '\n';
	}
	else	std::cout << "\n<html><head>"COMMON_CSS"</head>"INVALID_MTH"</html>\n";
}
