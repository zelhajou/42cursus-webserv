CURRENT_DIR := $(shell pwd)/
CXX = c++ -g -fsanitize=address 
#CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinclude -D PROJECT_PATH=\"$(CURRENT_DIR)\"
CXXFLAGS = -std=c++98 -Iinclude -D PROJECT_PATH=\"$(CURRENT_DIR)\"
SRC_DIR = src
BUILD_DIR = build
TARGET = webserv
CONDIRS = config/cgi_comm config/sockets config/html_default_error_files config/html_generated_files
APP = www/cgi_scripts/py_login_app/database www/cgi_scripts/py_login_app/sessions www/cgi_scripts/py_login_app/uploads
DEBUG ?= 0

ifeq ($(DEBUG), 1)
	CXXFLAGS += -D DEBUG=1
else
	CXXFLAGS += -D DEBUG=0
endif

SRCS = $(wildcard $(SRC_DIR)/**/*.cpp $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

all: mkdir $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

rmdi:
	@rm -rf $(CONDIRS)
	@rm -rf $(APP)

mkdir:
	@mkdir -p $(CONDIRS)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

fclean: rmdi clean

re: fclean all

.PHONY: all clean fclean re mkdir
