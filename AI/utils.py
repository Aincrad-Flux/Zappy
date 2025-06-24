#!/usr/bin/env python3
##
## EPITECH PROJECT, 2025
## Zappy
## File description:
## Utility functions for the AI client
##

from AI.logger import get_logger

utils_logger = get_logger(bot_id=0, team_name="utils", log_to_file=True, log_to_console=True)

def safe_convert(val, to_type: type, logger=None):
    """
    Safely convert a value to a specified type.

    This utility function attempts to convert a value to the specified type.
    If the conversion fails, it logs a critical error message and returns None
    without crashing the program.

    Args:
        val (any): The value to be converted
        to_type (type): The target type for conversion (e.g., int, str, float)
        logger (ZappyLogger, optional): A logger instance to use. Defaults to None.

    Returns:
        The converted value if successful, None if conversion fails
    """
    # Use provided logger or default to utils_logger
    log = logger or utils_logger

    try:
        return to_type(val)
    except Exception as e:
        log.critical(f"Safe convert failed: could not convert '{val}' to {to_type.__name__}: {str(e)}")
        return None

def format_inventory(inventory: dict, logger=None) -> str:
    """
    Formats an inventory dictionary into a readable string.

    Args:
        inventory (dict): A dictionary containing inventory items
        logger (ZappyLogger, optional): A logger instance to use. Defaults to None.

    Returns:
        str: A formatted string representation of the inventory
    """
    log = logger or utils_logger

    try:
        items = []
        for key, value in inventory.items():
            items.append(f"{key}: {value}")
        return ", ".join(items)
    except Exception as e:
        log.error(f"Error formatting inventory: {str(e)}")
        return str(inventory)

def log_command_result(command, result, success, logger=None):
    """
    Logs the result of a server command.

    Args:
        command (str): The command that was sent
        result (str): The response from the server
        success (bool): Whether the command was successful
        logger (ZappyLogger, optional): A logger instance to use. Defaults to None.
    """
    log = logger or utils_logger

    if success:
        log.debug(f"Command '{command.strip()}' succeeded with result: {result}")
    else:
        log.warning(f"Command '{command.strip()}' failed with result: {result}")

def get_resource_priority(level):
    """
    Returns a priority list of resources needed for the current level.

    Args:
        level (int): The current level of the AI

    Returns:
        list: A list of resources ordered by priority
    """
    # This could be expanded based on game strategy
    if level == 1:
        return ["linemate", "food"]
    elif level == 2:
        return ["linemate", "deraumere", "sibur", "food"]
    elif level == 3:
        return ["linemate", "sibur", "phiras", "food"]
    elif level == 4:
        return ["linemate", "deraumere", "sibur", "phiras", "food"]
    elif level == 5:
        return ["linemate", "deraumere", "sibur", "mendiane", "food"]
    elif level == 6:
        return ["linemate", "deraumere", "sibur", "phiras", "food"]
    elif level == 7:
        return ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame", "food"]
    else:
        return ["food"]
