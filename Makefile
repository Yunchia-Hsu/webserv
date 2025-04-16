CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -fsanitize=address -g
LDFLAGS = -lstdc++fs -fsanitize=address  # optional for older GCCs
NAME = webserved

SRC = src/main.cpp src/confiParser.cpp src/RouteConf.cpp \
	  src/ServerConf.cpp src/Served.cpp src/ClientConnection.cpp \
	  src/cgi.cpp src/io.cpp src/location.cpp src/request.cpp src/response.cpp src/utils.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
