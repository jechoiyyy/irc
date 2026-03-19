NAME		= ircserv
BOT_NAME	= ircbot

CPP			= c++
CPPFLAGS	= -Wall -Wextra -Werror -std=c++98 -Iinclude -Ibot -MMD -MP

OBJ_DIR = obj

FILES		= Message.cpp Client.cpp CommandHandler.cpp Server.cpp Channel.cpp main.cpp
SRCS		= $(addprefix src/, $(FILES))
OBJS		= $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

BOT_FILES	= Bot.cpp main.cpp
BOT_SRCS	= $(addprefix bot/, $(BOT_FILES))
BOT_OBJS	= $(addprefix $(OBJ_DIR)/, $(BOT_SRCS:.cpp=.o))

DEPS		= $(OBJS:.o=.d) $(BOT_OBJS:.o=.d)

all: $(NAME)

bot: $(BOT_NAME)

$(NAME): $(OBJS)
	$(CPP) $(CPPFLAGS) $^ -o $@

$(BOT_NAME): $(BOT_OBJS)
	$(CPP) $(CPPFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) $(BOT_NAME)

re: fclean all

.PHONY: all bot clean fclean re
