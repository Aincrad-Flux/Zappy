#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Zappy AI client
"""

import argparse
import socket
import sys
import time
import os
import sys

# Add the parent directory to the path so we can import the ai module
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from zappy_ai.ai import ZappyAI

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

    return parser.parse_args()

def connect_to_server(host, port):
    """Connect to the Zappy server"""
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((host, port))
        return client_socket
    except socket.error as e:
        print(f"Connection error: {e}")
        sys.exit(1)

def main():
    """Main function"""
    args = parse_arguments()

    # Connect to the server
    client_socket = connect_to_server(args.host, args.port)

    # Wait for "WELCOME" message
    welcome_msg = client_socket.recv(1024).decode('utf-8').strip()
    if welcome_msg != "WELCOME":
        print(f"Expected 'WELCOME', got '{welcome_msg}'")
        sys.exit(1)

    # Send team name
    client_socket.sendall(f"{args.name}\n".encode('utf-8'))

    # Wait for client num and map dimensions
    response = client_socket.recv(1024).decode('utf-8').strip()
    if "ko" in response.lower():
        print(f"Server rejected team name: {response}")
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

        print(f"Connected to server as client #{client_num}")
        print(f"Map dimensions: {width}x{height}")

        # Create and start the AI
        ai = ZappyAI(client_socket, client_num, width, height)
        ai.run()

    except (ValueError, IndexError) as e:
        print(f"Error parsing server response: {e}")
        sys.exit(1)
    finally:
        client_socket.close()

if __name__ == "__main__":
    main()
