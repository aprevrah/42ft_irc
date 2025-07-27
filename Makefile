NAME = ircserv
SRCS = src/main.cpp \
src/Server.cpp \
src/Client.cpp \
src/Command.cpp \
src/Channel.cpp \
src/ChannelManager.cpp \
src/log.cpp \
src/commands/cmd_cap.cpp \
src/commands/cmd_nick.cpp \
src/commands/cmd_pass.cpp \
src/commands/cmd_user.cpp \
src/commands/cmd_ping.cpp \
src/commands/cmd_join.cpp \
src/commands/cmd_part.cpp \
src/commands/cmd_quit.cpp \
src/commands/cmd_privmsg.cpp \
src/commands/cmd_mode.cpp \
src/commands/cmd_invite.cpp \
src/commands/cmd_topic.cpp

SRC_DIR = src
OBJ_DIR = obj
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I./includes -g #TODO: remove -g
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
