##
## Makefile for Zappy AI
##

NAME = zappy_ai
MAIN_FILE = main.py
PYTHON = python3
PIP = pip3
DIST_DIR = dist
BUILD_DIR = build
SPEC_DIR = spec

# Module files (new modular structure)
MODULES = ai_core.py models.py utils.py communication.py actions.py strategy.py

# Required Python packages
REQUIREMENTS = pyinstaller

.PHONY: all clean fclean re zappy_ai install check

all: $(NAME)

# Add check target to verify all modules compile correctly
check:
	@echo "Checking Python syntax..."
	@$(PYTHON) -m py_compile $(MAIN_FILE) $(MODULES)
	@echo "All modules syntax OK"

# Update zappy_ai target to depend on check
zappy_ai: install check
	@echo "Building $(NAME) binary..."
	$(PYTHON) -m PyInstaller --onefile --name=$(NAME) $(MAIN_FILE)
	@if [ -f $(DIST_DIR)/$(NAME) ]; then \
		cp $(DIST_DIR)/$(NAME) ./$(NAME); \
		chmod +x ./$(NAME); \
		echo "Binary $(NAME) created successfully!"; \
	else \
		echo "Failed to build $(NAME)"; \
		exit 1; \
	fi

install:
	@echo "Installing required packages..."
	$(PIP) install $(REQUIREMENTS)

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR) $(DIST_DIR) $(NAME).spec __pycache__ */__pycache__

fclean: clean
	@echo "Removing binary..."
	rm -f ./$(NAME)

re: fclean all
