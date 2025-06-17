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
PYTHON_IA_ENTRY = ia/src/main.py
PYTHON_IA_BIN = zappy_ia

# Couleurs
GREEN = \033[0;32m
YELLOW = \033[0;33m
RED = \033[0;31m
NC = \033[0m # No Color

# Règles principales
all: $(SERVER_BIN) $(GUI_BIN) $(AI_BIN) $(PYTHON_IA_BIN)

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

$(PYTHON_IA_BIN):
	@command -v pyinstaller >/dev/null 2>&1 || { \
		echo "$(RED)[ERROR] PyInstaller is not installed. Use: pip install pyinstaller$(NC)"; exit 1; }
	@echo "$(YELLOW)[PYTHON] Building Python IA executable...$(NC)"
	@pyinstaller --onefile --name $(PYTHON_IA_BIN) --distpath . --workpath obj/py --specpath obj/py $(PYTHON_IA_ENTRY)
	@echo "$(GREEN)[PYTHON] Executable $(PYTHON_IA_BIN) built successfully!$(NC)"

# Compilation des fichiers objets
$(OBJ_DIR)/server/%.o: $(SERVER_DIR)/%.c
	@mkdir -p $(OBJ_DIR)/server
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/gui/%.o: $(GUI_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)/gui
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/ai/%.o: $(AI_DIR)/%.c
	@mkdir -p $(OBJ_DIR)/ai
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@$(CC) $(CFLAGS) -c $< -o $@

# Règles spécifiques
zappy_server: $(SERVER_BIN)
zappy_gui: $(GUI_BIN)
zappy_ai: $(AI_BIN)

# Nettoyage
clean:
	@echo "$(RED)Cleaning object files...$(NC)"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "$(RED)Cleaning binaries...$(NC)"
	@rm -f $(SERVER_BIN) $(GUI_BIN) $(AI_BIN) $(PYTHON_IA_BIN)
	@rm -rf obj/py __pycache__ zappy_ia.spec build

re: fclean all

debug: CFLAGS += -g3 -DDEBUG
debug: CXXFLAGS += -g3 -DDEBUG
debug: all

.PHONY: all clean fclean re debug install-deps tests_run help init zappy_server zappy_gui zappy_ai zappy_ia
