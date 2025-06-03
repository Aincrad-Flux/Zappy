##
## EPITECH PROJECT, 2024
## zappy
## File description:
## Makefile
##

# Compilateurs
CC = gcc
CXX = g++

# Options de compilation
CFLAGS = -Wall -Wextra -std=c99
CXXFLAGS = -Wall -Wextra -std=c++17 -I./gui
LDFLAGS =

# Bibliothèques
LIBS_SERVER =
# Detect OS for GUI libs
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LIBS_GUI = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif
ifeq ($(UNAME_S),Darwin)
    LIBS_GUI = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
endif
LIBS_AI =

# Répertoires
SRC_DIR = src
SERVER_DIR = $(SRC_DIR)/server
GUI_DIR = gui
AI_DIR = $(SRC_DIR)/ai
OBJ_DIR = obj
INCLUDE_DIR = include

# Fichiers sources
SERVER_SRC = $(wildcard $(SERVER_DIR)/*.c)
GUI_SRC = $(wildcard $(GUI_DIR)/*.cpp)
AI_SRC = $(wildcard $(AI_DIR)/*.c)

# Fichiers objets
SERVER_OBJ = $(SERVER_SRC:$(SERVER_DIR)/%.c=$(OBJ_DIR)/server/%.o)
GUI_OBJ = $(GUI_SRC:$(GUI_DIR)/%.cpp=$(OBJ_DIR)/gui/%.o)
AI_OBJ = $(AI_SRC:$(AI_DIR)/%.c=$(OBJ_DIR)/ai/%.o)

# Binaires
SERVER_BIN = zappy_server
GUI_BIN = zappy_gui
AI_BIN = zappy_ai

# Couleurs pour l'affichage
GREEN = \033[0;32m
YELLOW = \033[0;33m
RED = \033[0;31m
NC = \033[0m # No Color

# Règles principales
all: $(SERVER_BIN) $(GUI_BIN) $(AI_BIN)

$(SERVER_BIN): $(SERVER_OBJ)
	@echo "$(YELLOW)Linking $(SERVER_BIN)...$(NC)"
	@$(CC) $(SERVER_OBJ) -o $(SERVER_BIN) $(LDFLAGS) $(LIBS_SERVER)
	@echo "$(GREEN)$(SERVER_BIN) compiled successfully!$(NC)"

$(GUI_BIN): $(GUI_OBJ)
	@echo "$(YELLOW)Linking $(GUI_BIN)...$(NC)"
	@$(CXX) $(GUI_OBJ) -o $(GUI_BIN) $(LDFLAGS) $(LIBS_GUI)
	@echo "$(GREEN)$(GUI_BIN) compiled successfully!$(NC)"

$(AI_BIN): $(AI_OBJ)
	@echo "$(YELLOW)Linking $(AI_BIN)...$(NC)"
	@$(CC) $(AI_OBJ) -o $(AI_BIN) $(LDFLAGS) $(LIBS_AI)
	@echo "$(GREEN)$(AI_BIN) compiled successfully!$(NC)"

# Règles individuelles pour chaque binaire
zappy_server: $(SERVER_BIN)

zappy_gui: $(GUI_BIN)

zappy_ai: $(AI_BIN)

# Compilation des fichiers objets du serveur
$(OBJ_DIR)/server/%.o: $(SERVER_DIR)/%.c
	@mkdir -p $(OBJ_DIR)/server
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compilation des fichiers objets de l'interface graphique
$(OBJ_DIR)/gui/%.o: $(GUI_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)/gui
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Compilation des fichiers objets de l'IA
$(OBJ_DIR)/ai/%.o: $(AI_DIR)/%.c
	@mkdir -p $(OBJ_DIR)/ai
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage
clean:
	@echo "$(RED)Cleaning object files...$(NC)"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "$(RED)Cleaning binaries...$(NC)"
	@rm -f $(SERVER_BIN) $(GUI_BIN) $(AI_BIN)

re: fclean all

# Tests et debug
debug: CFLAGS += -g3 -DDEBUG
debug: CXXFLAGS += -g3 -DDEBUG
debug: all

.PHONY: all clean fclean re debug install-deps tests_run help init zappy_server zappy_gui zappy_ai