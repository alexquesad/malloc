# Host type detection
ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

# Variables
NAME = libft_malloc_$(HOSTTYPE).so
LINK = libft_malloc.so

CC = gcc
CFLAGS = -Wall -Wextra -Werror -fPIC
LDFLAGS = -shared

# Directories
INC_DIR = includes
SRC_DIR = srcs
OBJ_DIR = objs

# Source files
SRCS = malloc.c free.c realloc.c show_alloc_mem.c \
       zone_manager.c block_manager.c utils.c

# Object files
OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

# Include paths
INCLUDES = -I$(INC_DIR)

# Colors
GREEN = \033[0;32m
RED = \033[0;31m
RESET = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(GREEN)Creating shared library $(NAME)...$(RESET)"
	@$(CC) $(LDFLAGS) -o $(NAME) $(OBJS)
	@ln -sf $(NAME) $(LINK)
	@echo "$(GREEN)Created symbolic link $(LINK) -> $(NAME)$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Add dependency on header file
$(OBJS): $(INC_DIR)/malloc.h

clean:
	@echo "$(RED)Cleaning object files...$(RESET)"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "$(RED)Removing library...$(RESET)"
	@rm -f $(NAME) $(LINK)

re: fclean all

# Testing
test: all
	@echo "$(GREEN)Running tests...$(RESET)"
	@$(CC) -g -o test_malloc test/test.c -L. -lft_malloc -Wl,-rpath,.
	@./test_malloc

.PHONY: all clean fclean re test