##
## EPITECH PROJECT, 2025
## zappy
## File description:
## Makefile for the Zappy AI client
##

NAME = zappy_ai
MAIN_FILE = main.py
PYTHON = python3

all: $(NAME)

$(NAME):
	@echo "Building $(NAME)..."
	@echo '#!/usr/bin/env python3' > $(NAME)
	@echo 'import os' >> $(NAME)
	@echo 'import sys' >> $(NAME)
	@echo 'script_dir = os.path.dirname(os.path.abspath(__file__))' >> $(NAME)
	@echo 'parent_dir = os.path.dirname(script_dir)' >> $(NAME)
	@echo 'sys.path.append(parent_dir)' >> $(NAME)
	@echo 'from AI.main import main' >> $(NAME)
	@echo 'from AI.network_client import NetworkClient' >> $(NAME)
	@echo 'if __name__ == "__main__":' >> $(NAME)
	@echo '    hostname, port, name, bot_id = main()' >> $(NAME)
	@echo '    client = NetworkClient(hostname, port, name, bot_id)' >> $(NAME)
	@echo '    client.establish_connection()' >> $(NAME)
	@echo '    client.run_client()' >> $(NAME)
	@echo '    client.check_server_capacity()' >> $(NAME)
	@echo '    client.disconnect()' >> $(NAME)
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
	@rm -f $(NAME)
	@echo "Done with full clean."

re: fclean all

re: fclean all

.PHONY: all clean fclean re