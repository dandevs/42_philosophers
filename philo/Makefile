# Executable name
NAME		= philo

# Compiler and flags
CC			= cc
# CFLAGS		= -Wall -Wextra -Werror -pthread
CFLAGS		=
RM			= rm -f

# Source files - found dynamically using shell commands
# Finds all .c files in src/ directory
SRC_DIR		= src
SRCS		:= $(shell find $(SRC_DIR) -type f -name "*.c")

# Header files - found dynamically using shell commands
# Finds all .h files in src/ directory
HEADERS		:= $(shell find $(SRC_DIR) -type f -name "*.h")

# Object files
OBJS_DIR	= obj
OBJS		:= $(patsubst $(SRC_DIR)/%.c,$(OBJS_DIR)/%.o,$(SRCS))

# Include path
INCLUDES	= -I$(SRC_DIR)

# Default target
all:			$(NAME)

# Link executable
$(NAME):		$(OBJS)
			$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

# Compile source files to object files
$(OBJS_DIR)/%.o:	$(SRC_DIR)/%.c $(HEADERS)
			@mkdir -p $(dir $@)
			$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Remove object files
clean:
			$(RM) -r $(OBJS_DIR)

# Remove object files and executable
fclean:			clean
			$(RM) $(NAME)

# Rebuild everything
re:			fclean all

# Debug build with symbols and no optimization
debug:			CFLAGS += -g -O0
debug:			all

# Force recompile
.PHONY:			all clean fclean re debug
