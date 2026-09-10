NAME		= philo
CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
RM			= rm -f

SRC_DIR		= src
SRCS		:= $(shell find $(SRC_DIR) -type f -name "*.c")
HEADERS		:= $(shell find $(SRC_DIR) -type f -name "*.h")

OBJS_DIR	= obj
OBJS		:= $(patsubst $(SRC_DIR)/%.c,$(OBJS_DIR)/%.o,$(SRCS))

INCLUDES	= -I$(SRC_DIR)

all:			$(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) -r $(OBJS_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

debug:			CFLAGS += -g -O0
debug:			all
.PHONY:			all clean fclean re debug
