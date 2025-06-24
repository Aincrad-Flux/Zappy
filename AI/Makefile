##
## EPITECH PROJECT, 2025
## zappy
## File description:
## Makefile for the Zappy AI client
##

NAME = zappy_ai
MAIN_FILE = zappy_ai
PYTHON = python3

all: $(NAME)

$(NAME):
	@echo "Building $(NAME)..."
	@chmod +x $(NAME)
	@cp $(NAME) ../$(NAME)
	@echo "Done! The AI client is ready to use."

clean:
	@echo "Cleaning Python cache files..."
	@find . -name '__pycache__' -type d -exec rm -rf {} +
	@find . -name '*.pyc' -delete
	@echo "Done cleaning."

fclean: clean
	@echo "Full clean: removing logs and executable..."
	@rm -rf logs/*.log
	@rm -f ../$(NAME)
	@echo "Done with full clean."

re: fclean all

re: fclean all

.PHONY: all clean fclean re