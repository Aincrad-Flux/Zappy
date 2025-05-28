# Makefile pour Zappy GUI avec Raylib

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
# Detect OS
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif
ifeq ($(UNAME_S),Darwin)
    LDFLAGS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
endif

# Répertoires
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Fichiers source
SOURCES = main.cpp Game.cpp Map.cpp Player.cpp Resource.cpp UI.cpp
OBJECTS = $(SOURCES:%.cpp=$(OBJ_DIR)/%.o)

# Nom de l'exécutable
TARGET = zappy_gui

# Règles
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re install-raylib install-raylib-mac setup-raylib

