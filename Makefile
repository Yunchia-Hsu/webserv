#THIS IS INKA'S TEST MAKEFILE

CXX = c++
FLAGS = -Wall -Wextra -Werror -std=c++11
NAME = parparpar

SRC = src/main.cpp src/confiParser.cpp src/RouteConf.cpp src/ServerConf.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(FLAGS) -o $(NAME) $(OBJ)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
