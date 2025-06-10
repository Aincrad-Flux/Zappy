##
## EPITECH PROJECT, 2024
## zappy
## File description:
## Makefile pour Zappy GUI avec Raylib
##

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 -I./include
# Detect OS
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Linux)
    ifeq ($(UNAME_M),x86_64)
        LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    else
        CXX = x86_64-linux-gnu-g++
        LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif
endif
ifeq ($(UNAME_S),Darwin)
    LDFLAGS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
endif

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
CONSOLE_OBJ_DIR = obj/console

# GUI Sources
SOURCES = $(filter-out $(SRC_DIR)/zappy_console.cpp, $(wildcard $(SRC_DIR)/*.cpp))
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))

# Console Sources
CONSOLE_SOURCES = $(SRC_DIR)/zappy_console.cpp $(SRC_DIR)/NetworkManager.cpp $(SRC_DIR)/Logger.cpp
CONSOLE_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(CONSOLE_OBJ_DIR)/%.o,$(CONSOLE_SOURCES))

TARGET = zappy_gui
CONSOLE_TARGET = zappy_console

all: $(TARGET)

console: $(CONSOLE_TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(CONSOLE_TARGET): $(CONSOLE_OBJECTS)
	$(CXX) $(CONSOLE_OBJECTS) -o $(CONSOLE_TARGET) -lpthread

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(CONSOLE_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET)
	rm -f $(CONSOLE_TARGET)

fclean: clean
	rm -f $(TARGET)
	rm -f $(CONSOLE_TARGET)
	rm *.log

re: fclean all

.PHONY: all clean fclean re install-raylib install-raylib-mac setup-raylib console
