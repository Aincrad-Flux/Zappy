##
## EPITECH PROJECT, 2025
## Makefile
## File description:
## Principal Makefile
##

NAME = zappy_server

all:
	@echo "Building server..."
	@$(MAKE) -C SERVER
	@cp SERVER/$(NAME) ./$(NAME)

clean:
	@echo "Cleaning server..."
	@$(MAKE) -C SERVER clean

fclean: clean
	@echo "Removing binary..."
	@$(MAKE) -C SERVER fclean
	@rm -f $(NAME)

re: fclean all
	@echo "Recompiling..."

.PHONY: all clean fclean re
