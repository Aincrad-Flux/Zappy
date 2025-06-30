##
## EPITECH PROJECT, 2025
## Makefile
## File description:
## Server Makefile
##

NAME = zappy_server
CFLAGS = -W -Wall -Wpedantic -g
LDFLAGS = -lm
INCLUDES = -I./include

SRC = 	src/server_init.c \
	src/client_handling.c \
	src/server_main.c \
	src/player.c \
	src/team.c \
	src/graphic.c \
	src/command/command.c \
	src/command/command_server/take.c \
	src/command/command_server/forward.c \
	src/command/command_server/inventory.c \
	src/command/command_server/set.c \
	src/command/command_server/incantation.c \
	src/command/command_server/eject.c \
	src/command/command_server/fork.c \
	src/command/command_server/Look.c \
	src/command/command_gui/commands_gui_egg.c \
	src/command/command_gui/commands_gui_extended_2.c \
	src/command/command_gui/commands_gui_extended.c \
	src/command/command_gui/commands_gui_game.c \
	src/command/command_gui/commands_gui_infos.c \
	src/command/command_gui/commands_gui_map.c \
	src/command/command_gui/commands_gui_player.c \
	src/command/command_gui/commands_gui_processor.c \
	src/command/command_gui/commands_gui_server.c \
	src/command/command_gui/commands_gui_team.c \
	src/command/command_gui/commands_gui_utils.c \
	src/command/command_gui/broadcast_gui_clients.c \
	src/utils/action.c \
	src/map/map.c \
	src/map/resource.c \
	src/time/tick.c

OBJ = $(SRC:src/%.c=obj/%.o)
OBJDIR = obj

all: $(NAME)

$(NAME): $(OBJ)
	@echo "Compiling binary..."
	@gcc $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	@gcc $(CFLAGS) $(INCLUDES) -c $< -o $@

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
