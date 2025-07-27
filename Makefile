NAME = ircserv
SRCS = src/main.cpp \
src/Server.cpp \
src/Client.cpp \
src/Command.cpp \
src/Channel.cpp \
src/ChannelManager.cpp \
src/log.cpp

SRC_DIR = src
OBJ_DIR = obj
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I./includes -g #TODO: remove -g
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
	rmdir $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
