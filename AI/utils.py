#!/usr/bin/env python3

"""
Utility functions for the Zappy AI
"""

import os
import datetime
import logging
from logging.handlers import RotatingFileHandler
import base64

# Configure logging directory to be in the current working directory
LOG_DIR = os.path.join(os.getcwd(), "logs")
os.makedirs(LOG_DIR, exist_ok=True)

def setup_logger(client_num, team_name, terminal_ui=False):
    """Setup a logger for a specific AI client"""
    # Create a unique log file for this AI client
    timestamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
    log_filename = f"ai_{team_name}_{client_num}_{timestamp}.log"
    log_path = os.path.join(LOG_DIR, log_filename)

    # Only print debug info if not in terminal UI mode
    if not terminal_ui:
        print(f"Setting up logger for AI client #{client_num}, team {team_name}")
        print(f"Log file path: {log_path}")

    # Ensure logs directory exists
    if not os.path.exists(LOG_DIR):
        if not terminal_ui:
            print(f"Log directory does not exist, creating: {LOG_DIR}")
        os.makedirs(LOG_DIR, exist_ok=True)

    # Configure the logger
    logger = logging.getLogger(f"ai_{client_num}")
    logger.setLevel(logging.DEBUG)

    # Clear any existing handlers
    if logger.handlers:
        for handler in logger.handlers:
            logger.removeHandler(handler)

    # Create a file handler with rotation (10 MB max size, keep 5 backup files)
    try:
        handler = RotatingFileHandler(log_path, maxBytes=10*1024*1024, backupCount=5)
        formatter = logging.Formatter('%(asctime)s [%(levelname)s] %(message)s')
        handler.setFormatter(formatter)
        if not terminal_ui:
            print(f"Successfully created log file handler for {log_path}")
    except Exception as e:
        if not terminal_ui:
            print(f"Error creating log file handler: {e}")
        # Create a fallback log in the current directory
        fallback_path = os.path.join(os.getcwd(), log_filename)
        if not terminal_ui:
            print(f"Trying fallback log path: {fallback_path}")
        handler = RotatingFileHandler(fallback_path, maxBytes=10*1024*1024, backupCount=5)
        formatter = logging.Formatter('%(asctime)s [%(levelname)s] %(message)s')
        handler.setFormatter(formatter)

    # Add the file handler to the logger
    logger.addHandler(handler)

    # Add a console handler only if not in terminal UI mode
    if not terminal_ui:
        console_handler = logging.StreamHandler()
        console_handler.setFormatter(formatter)
        logger.addHandler(console_handler)

    return logger

def encrypt_message(message, key):
    """Simple encryption using XOR with a team-specific key"""
    if not message or not key:
        return message

    # Convert message and key to bytes
    msg_bytes = message.encode('utf-8')
    key_bytes = key.encode('utf-8')

    # Extend key to match message length
    extended_key = (key_bytes * (len(msg_bytes) // len(key_bytes) + 1))[:len(msg_bytes)]

    # XOR operation
    encrypted = bytes(a ^ b for a, b in zip(msg_bytes, extended_key))

    # Convert to base64 for safe transmission
    return base64.b64encode(encrypted).decode('utf-8')

def decrypt_message(encrypted_message, key):
    """Decrypt a message encrypted with the team-specific key"""
    if not encrypted_message or not key:
        return encrypted_message

    try:
        # Convert from base64
        encrypted_bytes = base64.b64decode(encrypted_message)
        key_bytes = key.encode('utf-8')

        # Extend key to match message length
        extended_key = (key_bytes * (len(encrypted_bytes) // len(key_bytes) + 1))[:len(encrypted_bytes)]

        # XOR operation to decrypt
        decrypted = bytes(a ^ b for a, b in zip(encrypted_bytes, extended_key))

        return decrypted.decode('utf-8')
    except:
        return encrypted_message  # Return as-is if decryption fails
