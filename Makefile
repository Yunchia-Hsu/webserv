CXX = c++
# <<<<<<< HEAD
# CXXFLAGS = -Wall -Wextra -Werror -std=c++17 # -fsanitize=address -g 
# LDFLAGS = -lstdc++fs -fsanitize=address  # optional for older GCCs
# NAME = webserved
# =======
# <<<<<<< HEAD
# CXXFLAGS = -Wall -Wextra -Werror -std=gnu++17 #-std=c++17 
# LDFLAGS = -lstdc++fs #-fsanitize=address  # optional for older GCCs
# NAME = webserv
# =======
CXXFLAGS = -Wall -Wextra -Werror -std=c++17
LDFLAGS = -lstdc++fs 
NAME = webserved
# >>>>>>> 282e34f (wait for merge and push)
# >>>>>>> 6d07567 (wait for merge and push)

SRC_DIR = src
OBJ_DIR = obj

SRC = src/main.cpp src/confiParser.cpp src/RouteConf.cpp \
	  src/ServerConf.cpp src/Served.cpp src/ClientConnection.cpp \
	  src/cgi.cpp src/io.cpp src/location.cpp src/request.cpp src/response.cpp src/utils.cpp

OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC)) #$(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all
