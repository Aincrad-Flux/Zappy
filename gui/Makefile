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

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))

TARGET = zappy_gui

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re install-raylib install-raylib-mac setup-raylib
