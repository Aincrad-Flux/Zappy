##
## EPITECH PROJECT, 2025
## Makefile
## File description:
## Principal Makefile
##

NAME = zappy_server
CFLAGS = -W -Wall -Wpedantic -lm

SRC = 	src/server/server.c	\
	src/server/player.c \
	src/server/team.c \
	src/server/command/command.c	\
	src/server/command/command_server/take.c	\
	src/server/command/command_server/forward.c	\
	src/server/command/command_server/inventory.c	\
	src/server/command/command_server/set.c	\
	src/server/command/command_server/incantation.c	\
	src/server/command/command_server/eject.c	\
	src/server/command/command_server/broadcast.c	\
	src/server/command/command_server/fork.c	\
	src/server/command/command_gui/gui_commands.c	\
	src/server/utils/action.c \
	src/server/map/map.c \
	src/server/map/resource.c \
	src/server/time/tick.c

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