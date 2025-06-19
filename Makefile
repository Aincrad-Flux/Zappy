##
## EPITECH PROJECT, 2024
## zappy
## File description:
## Makefile
##

CC = gcc
CXX = g++

CFLAGS = -Wall -Wextra -std=c99
CXXFLAGS = -Wall -Wextra -std=c++17
LDFLAGS =

LIBS_SERVER =
LIBS_AI =

SRC_DIR = src
SERVER_DIR = $(SRC_DIR)/server
AI_DIR = $(SRC_DIR)/ai
OBJ_DIR = obj
INCLUDE_DIR = include

SERVER_SRC = $(wildcard $(SERVER_DIR)/*.c)
AI_SRC = $(wildcard $(AI_DIR)/*.c)

SERVER_OBJ = $(SERVER_SRC:$(SERVER_DIR)/%.c=$(OBJ_DIR)/server/%.o)
AI_OBJ = $(AI_SRC:$(AI_DIR)/%.c=$(OBJ_DIR)/ai/%.o)

SERVER_BIN = zappy_server
GUI_BIN = zappy_gui
AI_BIN = zappy_ai
PYTHON_IA_ENTRY = ia/src/main.py
PYTHON_IA_BIN = zappy_ia

GREEN = \033[0;32m
YELLOW = \033[0;33m
RED = \033[0;31m
NC = \033[0m

all: $(SERVER_BIN) $(GUI_BIN) $(AI_BIN)

$(SERVER_BIN): $(SERVER_OBJ)
	@echo "$(YELLOW)Linking $(SERVER_BIN)...$(NC)"
	@$(CC) $(SERVER_OBJ) -o $(SERVER_BIN) $(LDFLAGS) $(LIBS_SERVER)
	@echo "$(GREEN)$(SERVER_BIN) compiled successfully!$(NC)"

$(GUI_BIN):
	@echo "$(YELLOW)Building GUI using GUI/Makefile...$(NC)"
	@$(MAKE) -C GUI
	@cp GUI/$(GUI_BIN) ./$(GUI_BIN)
	@echo "$(GREEN)$(GUI_BIN) compiled successfully!$(NC)"

$(AI_BIN):
	@echo "$(YELLOW)Building AI using AI/Makefile...$(NC)"
	@$(MAKE) -C AI
	@cp AI/$(AI_BIN) ./$(AI_BIN)
	@echo "$(GREEN)$(AI_BIN) compiled successfully!$(NC)"

$(OBJ_DIR)/server/%.o: $(SERVER_DIR)/%.c
	@mkdir -p $(OBJ_DIR)/server
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "$(RED)Cleaning object files...$(NC)"
	@rm -rf $(OBJ_DIR)
	@$(MAKE) -C GUI clean
	@$(MAKE) -C AI clean

fclean: clean
	@echo "$(RED)Cleaning binaries...$(NC)"
	@rm -f $(SERVER_BIN) $(GUI_BIN) $(AI_BIN)
	@$(MAKE) -C GUI fclean
	@$(MAKE) -C AI fclean

re: fclean all

debug: CFLAGS += -g3 -DDEBUG
debug: CXXFLAGS += -g3 -DDEBUG
debug: all

.PHONY: all clean fclean re debug install-deps tests_run help init zappy_server zappy_gui zappy_ai zappy_ia
