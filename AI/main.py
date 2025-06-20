#!/usr/bin/env python3

"""
Zappy AI client
"""

import argparse
import socket
import sys
import time
import os
import sys
import logging
import datetime
from logging.handlers import RotatingFileHandler

# Import our new modules
from utils import setup_logger
from ai_core import ZappyAI

# Configure main logger
LOG_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "logs")
os.makedirs(LOG_DIR, exist_ok=True)

# Main logger will be initialized properly in main()
main_logger = None

def setup_main_logger(terminal_ui=False):
    """Setup the main logger for the application"""
    timestamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
    log_filename = f"zappy_ai_main_{timestamp}.log"
    log_path = os.path.join(LOG_DIR, log_filename)

    if not terminal_ui:
        print(f"Setting up main logger")
        print(f"Main log file path: {log_path}")

    # Ensure logs directory exists
    if not os.path.exists(LOG_DIR):
        if not terminal_ui:
            print(f"Main log directory does not exist, creating: {LOG_DIR}")
        os.makedirs(LOG_DIR, exist_ok=True)

    # Configure the logger
    logger = logging.getLogger("zappy_ai_main")
    logger.setLevel(logging.DEBUG)

    # Clear any existing handlers
    if logger.handlers:
        for handler in logger.handlers:
            logger.removeHandler(handler)

    # Create a file handler and console handler
    try:
        file_handler = RotatingFileHandler(log_path, maxBytes=10*1024*1024, backupCount=5)
        if not terminal_ui:
            print(f"Successfully created main log file handler for {log_path}")
    except Exception as e:
        if not terminal_ui:
            print(f"Error creating main log file handler: {e}")
        # Create a fallback log in the current directory
        fallback_path = os.path.join(os.getcwd(), log_filename)
        if not terminal_ui:
            print(f"Trying fallback main log path: {fallback_path}")
        file_handler = RotatingFileHandler(fallback_path, maxBytes=10*1024*1024, backupCount=5)

    # Only add console handler if not in terminal UI mode
    console_handler = None
    if not terminal_ui:
        console_handler = logging.StreamHandler(sys.stdout)

    # Set formatter
    formatter = logging.Formatter('%(asctime)s [%(levelname)s] %(message)s')
    file_handler.setFormatter(formatter)

    # Add the handlers to the logger
    logger.addHandler(file_handler)

    # Add console handler only if not in terminal UI mode
    if not terminal_ui and console_handler:
        console_handler.setFormatter(formatter)
        logger.addHandler(console_handler)

    return logger

def parse_arguments():
    """Parse command line arguments"""
    # Create parser with add_help=False to disable the default -h/--help option
    parser = argparse.ArgumentParser(description='Zappy AI Client', add_help=False)

    # Add help manually with a different flag
    parser.add_argument('--help', action='help', default=argparse.SUPPRESS,
                      help='Show this help message and exit')

    parser.add_argument('-p', '--port', type=int, required=True, help='Port number')
    parser.add_argument('-n', '--name', type=str, required=True, help='Name of the team')
    parser.add_argument('-h', '--host', type=str, default='localhost',
                        help='Name or IP of the machine; localhost by default')
    parser.add_argument('-u', '--terminal-ui', action='store_true',
                        help='Enable the terminal user interface mode (disables console logging)')

    return parser.parse_args()

def connect_to_server(host, port):
    """Connect to the Zappy server"""
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((host, port))
        return client_socket
    except socket.error as e:
        if main_logger:
            main_logger.error(f"Connection error: {e}")
        else:
            print(f"Connection error: {e}")
        sys.exit(1)

def main():
    """Main function"""
    args = parse_arguments()

    # Setup main logger with terminal UI option
    global main_logger
    main_logger = setup_main_logger(args.terminal_ui)

    # Print startup info only if not in terminal UI mode
    if not args.terminal_ui:
        print(f"Connecting to server {args.host}:{args.port}")

    # Connect to the server
    client_socket = connect_to_server(args.host, args.port)

    # Wait for "WELCOME" message
    welcome_msg = client_socket.recv(1024).decode('utf-8').strip()
    if welcome_msg != "WELCOME":
        main_logger.error(f"Expected 'WELCOME', got '{welcome_msg}'")
        sys.exit(1)

    # Send team name
    client_socket.sendall(f"{args.name}\n".encode('utf-8'))
    main_logger.info(f"Team name sent: {args.name}")

    # Wait for client num and map dimensions
    response = client_socket.recv(1024).decode('utf-8').strip()
    if "ko" in response.lower():
        main_logger.error(f"Server rejected team name: {response}")
        sys.exit(1)

    try:
        # The response may contain both the client number and map dimensions
        # It can be in format like "4\n20 20" or just "4" (with dimensions coming later)
        lines = response.split('\n')
        client_num = int(lines[0])

        # Check if dimensions came in the same response
        if len(lines) > 1 and ' ' in lines[1]:
            dimensions = lines[1].split()
        else:
            # If not, wait for dimensions in next message
            dimensions_data = client_socket.recv(1024).decode('utf-8').strip()
            dimensions = dimensions_data.split()

        width, height = int(dimensions[0]), int(dimensions[1])

        main_logger.info(f"Connected to server as client #{client_num}")
        main_logger.info(f"Map dimensions: {width}x{height}")

        # Create and start the AI (pass terminal_ui parameter)
        ai = ZappyAI(client_socket, client_num, width, height, args.name, args.terminal_ui)
        ai.run()

    except (ValueError, IndexError) as e:
        main_logger.error(f"Error parsing server response: {e}")
        sys.exit(1)
    finally:
        if main_logger:
            main_logger.info("AI client shutting down")
        client_socket.close()

# Initialize the main logger
main_logger = setup_main_logger()

if __name__ == "__main__":
    main()
