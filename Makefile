##
## EPITECH PROJECT, 2024
## zappy
## File description:
## Makefile
##

SERVER_BIN = zappy_server
GUI_BIN = zappy_gui
AI_BIN = zappy_ai

GREEN = \033[0;32m
YELLOW = \033[0;33m
RED = \033[0;31m
NC = \033[0m

all: $(SERVER_BIN) $(GUI_BIN) $(AI_BIN)

$(SERVER_BIN):
	@echo "$(YELLOW)Building SERVER using SERVER/Makefile...$(NC)"
	@$(MAKE) -C SERVER
	@cp SERVER/$(SERVER_BIN) ./$(SERVER_BIN)
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


clean:
	@echo "$(RED)Cleaning object files...$(NC)"
	@$(MAKE) -C GUI clean
	@$(MAKE) -C AI clean
	@$(MAKE) -C SERVER clean
	@rm -f *.log

fclean: clean
	@echo "$(RED)Cleaning binaries...$(NC)"
	@$(MAKE) -C GUI fclean
	@$(MAKE) -C AI fclean
	@$(MAKE) -C SERVER fclean
	@rm -f $(SERVER_BIN) $(GUI_BIN) $(AI_BIN)

re: fclean all

debug: all

zappy_server: $(SERVER_BIN)

zappy_gui: $(GUI_BIN)

zappy_ai: $(AI_BIN)

.PHONY: all clean fclean re debug install-deps tests_run help init zappy_server zappy_gui zappy_ai zappy_ia
