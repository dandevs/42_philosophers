# Executable name
NAME		:= philo

# Compiler and flags
CC			:= cc
CFLAGS		:= -Wall -Wextra -Werror -pthread
RM			:= rm -f

# Source files - found dynamically using shell commands
# Finds all .c files in src/ directory
SRC_DIR		:= src
SRCS		:= $(shell find $(SRC_DIR) -type f -name "*.c")

# Header files - found dynamically using shell commands
# Finds all .h files in src/ directory
HEADERS		:= $(shell find $(SRC_DIR) -type f -name "*.h")

# Object files
OBJS_DIR	:= obj
OBJS		:= $(patsubst $(SRC_DIR)/%.c,$(OBJS_DIR)/%.o,$(SRCS))

# Include path
INCLUDES	:= -I$(SRC_DIR)

# Colors for output
GREEN		:= \033[0;32m
YELLOW		:= \033[0;33m
BLUE		:= \033[0;34m
RESET		:= \033[0m

# Default target
all:			$(NAME)

# Link executable
$(NAME):		$(OBJS)
			@echo "$(YELLOW)Linking $(NAME)...$(RESET)"
			$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
			@echo "$(GREEN)✓ $(NAME) compiled successfully!$(RESET)"

# Compile source files to object files
$(OBJS_DIR)/%.o:	$(SRC_DIR)/%.c $(HEADERS)
			@mkdir -p $(dir $@)
			@echo "$(BLUE)Compiling $<...$(RESET)"
			$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Remove object files
clean:
			@echo "$(YELLOW)Removing object files...$(RESET)"
			$(RM) -r $(OBJS_DIR)
			@echo "$(GREEN)✓ Object files removed!$(RESET)"

# Remove object files and executable
fclean:			clean
			@echo "$(YELLOW)Removing $(NAME)...$(RESET)"
			$(RM) $(NAME)
			@echo "$(GREEN)✓ $(NAME) removed!$(RESET)"

# Rebuild everything
re:			fclean all

# Debug build with symbols and no optimization
debug:			CFLAGS += -g -O0
debug:			all

# Print source files (useful for debugging)
print:
			@echo "Source files found:"
			@echo "$(SRCS)"
			@echo ""
			@echo "Header files found:"
			@echo "$(HEADERS)"
			@echo ""
			@echo "Object files to create:"
			@echo "$(OBJS)"

# Check for missing files
check:
			@if [ -z "$(SRCS)" ]; then \
				echo "$(YELLOW)Warning: No .c files found in $(SRC_DIR)/$(RESET)"; \
			else \
				echo "$(GREEN)✓ Found $(words $(SRCS)) source file(s)$(RESET)"; \
			fi
			@if [ -z "$(HEADERS)" ]; then \
				echo "$(YELLOW)Warning: No .h files found in $(SRC_DIR)/$(RESET)"; \
			else \
				echo "$(GREEN)✓ Found $(words $(HEADERS)) header file(s)$(RESET)"; \
			fi

# Force recompile
.PHONY:			all clean fclean re debug print check
