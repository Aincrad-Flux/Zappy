##
## EPITECH PROJECT, 2025
## Makefile
## File description:
## Principal Makefile
##

NAME = zappy_server
CFLAGS = -W -Wall -Wpedantic -Werror -lm

SRC = 	src/server/server.c	\
	src/server/player.c \
	src/server/team.c \
	src/server/utils/command.c	\
	src/server/utils/action.c
OBJ = $(SRC:src/%.c=obj/%.o)
OBJDIR = obj

all: $(LIBMY) $(NAME)
$(NAME): $(OBJ)
	@echo "Compiling binary..."
	@gcc $(CFLAGS) $^ -o $@

obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	@gcc $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning object files..."
	@rm -f $(OBJ)
	@rm -rf $(OBJDIR)

fclean: clean
	@echo "Removing binary..."
	@rm -f $(NAME)

re: fclean all
	@echo "Recompiling..."

.PHONY: all clean fclean re