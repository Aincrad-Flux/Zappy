#!/usr/bin/env python3
##
## EPITECH PROJECT, 2025
## Zappy
## File description:
## Logger module for the AI client
##

"""
Logger module for Zappy AI

This module provides a custom logging system for the Zappy AI project.
It configures different log levels, formats, and output destinations.
"""

import logging
import os
import sys
import time
from datetime import datetime

COLORS = {
    "RESET": "\033[0m",
    "DEBUG": "\033[36m",  # Cyan
    "INFO": "\033[32m",   # Green
    "WARNING": "\033[33m", # Yellow
    "ERROR": "\033[31m",  # Red
    "CRITICAL": "\033[41m\033[37m", # White on Red background
}


class ZappyLogger:
    """
    Custom logger for the Zappy AI project.

    This class provides methods for configuring and using a custom logger
    with different output formats, log levels, and destinations.
    """

    def __init__(self, bot_id=None, team_name=None, log_to_file=True, log_to_console=True, log_level=logging.INFO):
        """
        Initialize the ZappyLogger with custom configuration.

        Args:
            bot_id (int, optional): Bot identifier for log differentiation. Defaults to None.
            team_name (str, optional): Team name for log identification. Defaults to None.
            log_to_file (bool, optional): Enable logging to file. Defaults to True.
            log_to_console (bool, optional): Enable logging to console. Defaults to True.
            log_level (int, optional): Minimum log level to record. Defaults to logging.INFO.
        """
        self.logger = logging.getLogger(f"zappy_ai_{bot_id}")
        self.logger.setLevel(log_level)
        self.logger.propagate = False
        self.bot_id = bot_id
        self.team_name = team_name
        self.log_to_file = log_to_file
        self.log_to_console = log_to_console
        self.log_level = log_level

        if self.logger.hasHandlers():
            self.logger.handlers.clear()

        if log_to_console:
            console_handler = logging.StreamHandler(sys.stdout)
            console_handler.setLevel(log_level)
            console_formatter = self.ColoredFormatter('%(asctime)s [%(levelname)s] [Bot %(bot_id)s] %(message)s',
                                                     {"bot_id": bot_id or 0})
            console_handler.setFormatter(console_formatter)
            self.logger.addHandler(console_handler)

        if log_to_file:
            os.makedirs("logs", exist_ok=True)

            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            team_str = f"_{team_name}" if team_name else ""
            bot_str = f"_bot{bot_id}" if bot_id is not None else ""
            log_filename = f"logs/zappy_ai{team_str}{bot_str}_{timestamp}.log"

            file_handler = logging.FileHandler(log_filename)
            file_handler.setLevel(log_level)
            file_formatter = logging.Formatter('%(asctime)s [%(levelname)s] [Bot %(bot_id)s] %(message)s',
                                              "%Y-%m-%d %H:%M:%S")
            file_handler.setFormatter(file_formatter)
            self.logger.addHandler(file_handler)

    class ColoredFormatter(logging.Formatter):
        """A custom formatter for adding colors to log messages."""

        def __init__(self, fmt, defaults=None):
            super().__init__(fmt)
            self.defaults = defaults or {}

        def format(self, record):
            """Format the record with colors based on log level."""
            for key, value in self.defaults.items():
                if not hasattr(record, key):
                    setattr(record, key, value)

            levelname = record.levelname
            if levelname in COLORS:
                record.levelname = f"{COLORS[levelname]}{levelname}{COLORS['RESET']}"

            return super().format(record)

    def debug(self, message, *args, **kwargs):
        """Log a debug message."""
        self.logger.debug(message, *args, **kwargs)

    def info(self, message, *args, **kwargs):
        """Log an info message."""
        self.logger.info(message, *args, **kwargs)

    def warning(self, message, *args, **kwargs):
        """Log a warning message."""
        self.logger.warning(message, *args, **kwargs)

    def error(self, message, *args, **kwargs):
        """Log an error message."""
        self.logger.error(message, *args, **kwargs)

    def critical(self, message, *args, **kwargs):
        """Log a critical message."""
        self.logger.critical(message, *args, **kwargs)

    def log_state_change(self, old_state, new_state):
        """Log a state change in the AI core."""
        self.info(f"State change: {old_state} -> {new_state}")

    def log_action(self, action):
        """Log an action being performed by the AI."""
        self.debug(f"Performing action: {action.strip()}")

    def log_resource_found(self, resource):
        """Log when a resource is found."""
        self.info(f"Found resource: {resource}")

    def log_inventory(self, inventory):
        """Log the current inventory state."""
        self.debug(f"Inventory: {inventory}")

    def log_message(self, message, direction=None):
        """Log a message received from other players."""
        direction_str = f" from direction {direction}" if direction is not None else ""
        self.debug(f"Message received{direction_str}: {message}")

    def log_ritual_status(self, status):
        """Log changes in ritual status."""
        self.info(f"Ritual status: {status}")

    def log_level_up(self, old_level, new_level):
        """Log when the AI levels up."""
        self.info(f"Level up! {old_level} -> {new_level}")

    def log_network(self, message):
        """Log network-related events."""
        self.debug(f"Network: {message}")

default_logger = ZappyLogger()

def get_logger(bot_id=None, team_name=None, log_to_file=True, log_to_console=True, log_level=logging.INFO):
    """
    Get a configured ZappyLogger instance.

    Args:
        bot_id (int, optional): Bot identifier for log differentiation. Defaults to None.
        team_name (str, optional): Team name for log identification. Defaults to None.
        log_to_file (bool, optional): Enable logging to file. Defaults to True.
        log_to_console (bool, optional): Enable logging to console. Defaults to True.
        log_level (int, optional): Minimum log level to record. Defaults to logging.INFO.

    Returns:
        ZappyLogger: A configured logger instance
    """
    return ZappyLogger(
        bot_id=bot_id,
        team_name=team_name,
        log_to_file=log_to_file,
        log_to_console=log_to_console,
        log_level=log_level
    )
