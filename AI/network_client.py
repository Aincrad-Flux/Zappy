#!/usr/bin/env python3
##
## EPITECH PROJECT, 2025
## Zappy
## File description:
## Network client implementation for the AI
##

from http import client
from re import I
import socket
import os
from AI.utils import safe_convert
from AI.ai_core import AICore
from AI.logger import get_logger
import logging
import selectors
import random
import subprocess

class NetworkClient():
    """
    This class implements a client which uses selectors for non-blocking communication.

    The NetworkClient handles all communication with the server, including establishing
    connection, sending commands, receiving responses, and processing server messages.
    It also manages the AI agent's state based on server responses.
    """

    def __init__(self, hostname: str, port: str, team_name: str, bot_id: str, use_ui: bool = False):
        """
        Initialize the NetworkClient with server connection details.

        Args:
            hostname (str): IP address of the server
            port (str): Port number of the server
            team_name (str): Name of the team to connect to
            bot_id (str): Unique identifier for this bot instance
            use_ui (bool, optional): Whether terminal UI is enabled. Defaults to False.
        """
        self.team = team_name
        self.hostname = hostname
        self.port = port
        self.bot_id = int(bot_id)
        self.map_width = 0
        self.map_height = 0
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.selector = selectors.DefaultSelector()
        self.init_status = 0
        self.connected = 0
        self.use_ui = use_ui
        self.logger = get_logger(bot_id=int(bot_id), team_name=team_name, log_to_console=not use_ui)
        self.logger.info(f"NetworkClient initialized for team '{team_name}' with bot ID {bot_id}")
        self.agent = AICore(self.team, int(bot_id), self.use_ui)
        self.agent.bot_id = self.bot_id
        self.counter = 0

    def establish_connection(self):
        """
        Initialize the socket connection to the server and register selectors.

        This method sets up the socket for non-blocking operations, establishes
        connection with the server, and registers read/write events with the selector.

        Returns:
            None
        """
        self.socket.setblocking(False)
        port_num = safe_convert(self.port, int, logger=self.logger)
        if port_num is None:
            port_num = 4242  # Default port if conversion fails
            self.logger.warning(f"Invalid port number '{self.port}', using default: {port_num}")

        self.logger.info(f"Connecting to server at {self.hostname}:{port_num}")
        self.socket.connect_ex((self.hostname, port_num))
        events = selectors.EVENT_READ | selectors.EVENT_WRITE
        self.selector.register(self.socket, events)
        self.logger.debug("Socket registered with selector")

    def handle_server_info(self, message, stage):
        """
        Process and save initialization information received from the server.

        Args:
            message (str): Message received from the server
            stage (int): Current initialization stage (1: team slots, 2: map dimensions)

        Returns:
            None

        Note:
            Stage 1: Extracts free team slots information
            Other stages: Extracts map width and height and completes initialization
        """
        if stage == 1:
            slots = safe_convert(message, int, logger=self.logger)
            self.agent.free_slots = 0 if slots is None else slots
            self.logger.info(f"Team slots available: {self.agent.free_slots}")
            self.agent.action = ""
            self.init_status = 2
        else:
            message = message.split()
            width = safe_convert(message[0], int, logger=self.logger)
            height = safe_convert(message[1], int, logger=self.logger)
            self.map_width = 0 if width is None else width
            self.map_height = 0 if height is None else height
            self.logger.info(f"Map dimensions: {self.map_width}x{self.map_height}")
            self.agent.action = ""
            self.init_status = 3
            self.connected = 1
            self.agent.active = 1
            self.logger.info("Connection established, client ready")


    def run_client(self):
        """
        Main client loop that handles all communication with the server.

        This method continuously:
        - Reads data from the server
        - Processes received messages according to protocol
        - Updates the AI agent's state based on received information
        - Sends actions decided by the AI agent to the server

        Returns:
            None

        Note:
            This method runs indefinitely until the connection is terminated
            or an error occurs.
        """
        message = ""
        while True:
            event = self.selector.select(timeout=None)
            for _, mask in event:
                if mask & selectors.EVENT_READ:
                    try:
                        data = self.socket.recv(1024).decode("utf-8")
                        if data:
                            message += data
                        else:
                            self.logger.warning("Connection closed by the server")
                            self.selector.unregister(self.socket)
                            self.socket.close()
                            break
                    except Exception as e:
                        self.logger.error(f"Error receiving data: {e}")
                        self.selector.unregister(self.socket)
                        self.socket.close()
                        break
                    tmp = message.split("\n")
                    for elem in tmp[:-1]:
                        if "dead" in elem:
                            self.logger.critical(f"Bot died, id: {self.bot_id}")
                            exit(0)
                        if "WELCOME" in elem and self.connected == 0:
                            self.logger.info("Welcome message received, sending team name")
                            self.agent.action = (self.team + '\n')
                        elif self.init_status < 3:
                            if "message" in elem:
                                continue
                            self.logger.debug(f"Processing server info (stage {self.init_status}): {elem}")
                            self.handle_server_info(elem, self.init_status)
                        elif  "Elevation underway" in elem:
                            self.logger.log_ritual_status("Elevation underway")
                            self.agent.state = 8
                        elif "Current level:" in elem:
                            old_level = self.agent.level
                            level_str = ''.join(filter(str.isdigit, elem))
                            new_level = safe_convert(level_str, int, logger=self.logger)

                            if new_level is None:
                                self.logger.error(f"Failed to parse level from '{elem}'")
                            else:
                                self.agent.level = new_level

                                if self.agent.level == 8:
                                    self.logger.info("Maximum level reached! Game completed.")
                                    exit(0)

                                self.logger.log_level_up(old_level, self.agent.level)
                            self.agent.state = 9
                            self.agent.found_new_item = False
                            self.agent.target_resource = ""
                            self.agent.ritual_mode = 0
                            self.agent.ritual_leader = 0
                            self.agent.players_for_ritual = 1
                            self.agent.source_direction = 9
                            self.agent.ritual_ready = 0
                            self.agent.clear_read_flag = 0
                            self.agent.clear_message_flag = 0
                            self.agent.action = 0
                        elif self.init_status >= 3 and "message" in elem:
                            if self.agent.clear_read_flag == 1:
                                self.agent.clear_read_flag = 0
                                message = message.split("\n")[-1]
                                continue

                            self.logger.debug(f"Message received: {elem}")
                            self.agent.message_received = [elem]
                            self.agent.handle_message(elem)
                            message = message.split("\n")[-1]
                            continue
                        elif isinstance(self.agent.action, str) and self.agent.action.startswith("Take ") and "food" not in self.agent.action and elem == "ok":
                            resource = self.agent.action.split(" ")[1].strip()
                            self.logger.info(f"Successfully took resource: {resource}")
                            self.agent.refresh_team_inventory()
                            self.agent.found_new_item = True
                        elif self.agent.action == "Inventory\n":
                            try:
                                self.agent.parse_backpack(elem)
                            except ValueError:
                                self.logger.error(f"Error parsing inventory: {elem}")
                                pass
                        elif self.agent.action == "Look\n":
                            self.agent.vision = elem
                            self.logger.debug("Vision information received")
                        elif self.agent.action == "Connect_nbr\n":
                            slots = safe_convert(elem, int, logger=self.logger)
                            if slots is not None:
                                self.agent.free_slots = slots
                                self.logger.debug(f"Free team slots: {slots}")
                                if slots > 0 and self.bot_id < 6 and self.agent.fork == 1:
                                    script_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "zappy_ai")
                                    next_bot_id = str(self.bot_id + 1)
                                    self.logger.info(f"Forking new bot with ID {next_bot_id}")
                                    subprocess.Popen(["python3", script_path, "-p", self.port, "-n", self.team, "-i", next_bot_id])
                                    self.agent.fork = 0  # Only fork once per AI instance
                            else:
                                self.logger.error(f"Error parsing Connect_nbr response: {elem}")
                                self.agent.free_slots = 0
                        # Handling Fork response is no longer needed as we use the Connect_nbr method
                        message = message.split("\n")[-1]
                        self.agent.active = 1

                if mask & selectors.EVENT_WRITE:
                    if self.connected and self.agent.active == 1:
                        self.agent.decide_action()
                    if self.agent.action and self.agent.active != 0:
                        self.counter += 1
                        if self.agent.action == (self.team + '\n') and self.connected == 0:
                            self.init_status = 1
                        try:
                            if isinstance(self.agent.action, str):
                                self.logger.log_action(self.agent.action)
                                self.socket.send(self.agent.action.encode())
                            else:
                                self.logger.warning(f"Invalid action type: agent.action is not a string: {self.agent.action}")
                        except Exception as e:
                            self.logger.error(f"Error sending message: {e}")
                        self.agent.active = 0

    def disconnect(self):
        """
        Close the client connection and clean up resources.

        This method unregisters the socket from the selector and closes the socket.
        Any errors during disconnection are logged but do not stop program execution.

        Returns:
            None
        """
        try:
            self.logger.info("Disconnecting from server")
            self.selector.unregister(self.socket)
            self.socket.close()
        except Exception as e:
            self.logger.error(f"Error when disconnecting: {e}")

    def check_server_capacity(self):
        """
        Check if the server has available slots for new clients.

        This method examines the bot_id to determine if the server has
        capacity for this bot. Negative bot_id indicates no slots available.

        Returns:
            None
        """
        if self.bot_id < 0:
            self.logger.warning("The server does not have available slots")
