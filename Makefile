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
src/commands/cmd_topic.cpp \
src/commands/cmd_kick.cpp

SRC_DIR = src
OBJ_DIR = obj
CXX = clang
CXXFLAGS = -Wall -Wextra -Werror -I./includes -g -std=c++98 #TODO: remove -g
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# AFL++ fuzzing targets
AFL_CXX = afl-clang++
AFL_CXXFLAGS = -Wall -Wextra -Werror -I./includes -g -std=c++98
AFL_NAME = ircserv_fuzz
AFL_OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/afl_%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# AFL++ fuzzing build target
afl: $(AFL_OBJS)
	$(AFL_CXX) $(AFL_CXXFLAGS) $(AFL_OBJS) -o $(AFL_NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/afl_%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(AFL_CXX) $(AFL_CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) $(AFL_NAME)

re: fclean all

.PHONY: all clean fclean re afl
