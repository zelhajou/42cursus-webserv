CURRENT_DIR := $(shell pwd)/
CXX = c++
CXXFLAGS =  -Wall -Wextra -Werror -std=c++98 -Iinclude -D PROJECT_PATH=\"$(CURRENT_DIR)\" -D CGI_TIME_LIMIT=10
SRC_DIR = src
BUILD_DIR = build
NAME = webserv
CONDIRS = config/cgi_comm config/sockets config/html_default_error_files config/html_generated_files
APP = www/cgi_scripts/py_login_app/database www/cgi_scripts/py_login_app/sessions www/cgi_scripts/py_login_app/uploads
DEBUG ?= 0

ifeq ($(DEBUG), 1)
	CXXFLAGS += -D DEBUG=1
else
	CXXFLAGS += -D DEBUG=0
endif

SRCS = 	src/config/ConfigValidator.cpp src/http/Request.cpp           src/network/Sockets.cpp \
		src/config/Parser.cpp          src/http/Response.cpp          src/utils/TrieTree.cpp \
		src/config/Tokenizer.cpp       src/network/KQueue.cpp         src/utils/util.cpp \
		src/main.cpp

INC = 	include/ConfigStructures.hpp include/Parser.hpp           include/Sockets.hpp \
		include/ConfigValidator.hpp  include/Request.hpp          include/Tokenizer.hpp \
		include/KQueue.hpp           include/Response.hpp         include/TrieTree.hpp \


OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

all: mkdir $(NAME)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJS) $(INC)
	$(CXX) $(OBJS) -o $(NAME)

rmdi:
	@rm -rf $(CONDIRS)
	@rm -rf $(APP)

mkdir:
	@mkdir -p $(CONDIRS)

bonus: all

clean:
	rm -rf $(BUILD_DIR)

fclean: rmdi clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re mkdir
