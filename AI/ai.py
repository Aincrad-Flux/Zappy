#!/usr/bin/env python3

"""
Zappy AI implementation
"""

import random
import time
import socket
from enum import Enum
import threading
import queue
import re
import base64
import os
import datetime
import logging
from logging.handlers import RotatingFileHandler

# Configure logging directory
LOG_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "logs")
os.makedirs(LOG_DIR, exist_ok=True)
print(f"Log directory: {LOG_DIR}")

def setup_logger(client_num, team_name):
    """Setup a logger for a specific AI client"""
    # Create a unique log file for this AI client
    timestamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
    log_filename = f"ai_{team_name}_{client_num}_{timestamp}.log"
    log_path = os.path.join(LOG_DIR, log_filename)

    # Print debug info
    print(f"Setting up logger for AI client #{client_num}, team {team_name}")
    print(f"Log file path: {log_path}")

    # Ensure logs directory exists
    if not os.path.exists(LOG_DIR):
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
        print(f"Successfully created log file handler for {log_path}")
    except Exception as e:
        print(f"Error creating log file handler: {e}")
        # Create a fallback log in the current directory
        fallback_path = os.path.join(os.getcwd(), log_filename)
        print(f"Trying fallback log path: {fallback_path}")
        handler = RotatingFileHandler(fallback_path, maxBytes=10*1024*1024, backupCount=5)
        formatter = logging.Formatter('%(asctime)s [%(levelname)s] %(message)s')
        handler.setFormatter(formatter)

    # Add the file handler to the logger
    logger.addHandler(handler)

    # Add a console handler as well for direct visibility
    console_handler = logging.StreamHandler()
    console_handler.setFormatter(formatter)
    logger.addHandler(console_handler)

    return logger

# Added simple encryption utility for team communications
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

class ResourceType(Enum):
    """Resource types in the Zappy world"""
    FOOD = 0
    LINEMATE = 1
    DERAUMERE = 2
    SIBUR = 3
    MENDIANE = 4
    PHIRAS = 5
    THYSTAME = 6

class Direction(Enum):
    """Direction on the map"""
    NORTH = 1
    EAST = 2
    SOUTH = 3
    WEST = 4

class ZappyAI:
    """Implementation of the Zappy AI"""

    ELEVATION_REQUIREMENTS = {
        # Level: (players_needed, linemate, deraumere, sibur, mendiane, phiras, thystame)
        1: (1, 1, 0, 0, 0, 0, 0),
        2: (2, 1, 1, 1, 0, 0, 0),
        3: (2, 2, 0, 1, 0, 2, 0),
        4: (4, 1, 1, 2, 0, 1, 0),
        5: (4, 1, 2, 1, 3, 0, 0),
        6: (6, 1, 2, 3, 0, 1, 0),
        7: (6, 2, 2, 2, 2, 2, 1)
    }

    def __init__(self, socket, client_num, width, height, team_name=""):
        """Initialize the AI"""
        self.socket = socket
        self.client_num = client_num
        self.width = width
        self.height = height
        self.team_name = team_name

        # Player state
        self.level = 1
        self.direction = Direction.NORTH
        self.vision_range = 1  # Starts at 1, increases with level
        self.state = "COLLECTING_RESOURCES"
        self.running = True

        # Initialize the inventory
        self.inventory = {
            ResourceType.FOOD: 10,  # Start with some food
            ResourceType.LINEMATE: 0,
            ResourceType.DERAUMERE: 0,
            ResourceType.SIBUR: 0,
            ResourceType.MENDIANE: 0,
            ResourceType.PHIRAS: 0,
            ResourceType.THYSTAME: 0
        }

        # Set up target resources for elevation
        self.target_resources = {}
        self._update_target_resources()

        # Broadcasting configuration
        self.last_broadcast_time = 0
        self.broadcast_cooldown = 2  # Seconds between broadcasts
        self.encryption_key = f"{team_name}_key"  # Simple key for team

        # Team coordination
        self.team_members = set()
        self.team_member_positions = {}  # Store teammate positions
        self.incantation_participants = {}  # Players who've responded to elevation
        self.position = None  # Estimated position

        # Timeout handling
        self.elevation_attempt_time = 0
        self.elevation_timeout = 20  # How long to wait for team members before giving up

        # Communication queue for asynchronous server messages
        self.response_queue = queue.Queue()

        # Setup logging for this AI client
        self.logger = setup_logger(client_num, team_name)
        self.logger.info(f"AI client #{client_num} for team {team_name} initialized")
        self.logger.info(f"Map dimensions: {width}x{height}")  # Store positions of team members

        # Calculate resource requirements for next level
        self._update_target_resources()

        # Start the receiver thread
        self.receiver_thread = threading.Thread(target=self._response_receiver)
        self.receiver_thread.daemon = True
        self.receiver_thread.start()

        # Setup logger
        self.logger = setup_logger(client_num, team_name)
        self.logger.info("AI initialized")

    def _update_target_resources(self):
        """Update the target resources needed for the next level"""
        if self.level < 8:
            _, linemate, deraumere, sibur, mendiane, phiras, thystame = self.ELEVATION_REQUIREMENTS[self.level]
            self.target_resources = {
                ResourceType.LINEMATE: linemate,
                ResourceType.DERAUMERE: deraumere,
                ResourceType.SIBUR: sibur,
                ResourceType.MENDIANE: mendiane,
                ResourceType.PHIRAS: phiras,
                ResourceType.THYSTAME: thystame
            }
        else:
            self.target_resources = {}  # Max level reached

    def _send_command(self, command):
        """Send a command to the server and wait for response"""
        try:
            self.socket.sendall(f"{command}\n".encode('utf-8'))
            response = self.response_queue.get(timeout=10)
            return response
        except (socket.error, queue.Empty) as e:
            self.logger.error(f"Error sending command: {e}")
            return "error"

    def _response_receiver(self):
        """Thread to receive responses from the server"""
        buffer = ""
        while self.running:
            try:
                data = self.socket.recv(4096).decode('utf-8')
                if not data:
                    break

                buffer += data
                lines = buffer.split('\n')
                buffer = lines.pop()  # Keep the last incomplete line in the buffer

                for line in lines:
                    line = line.strip()
                    if line:
                        if line == "dead":
                            self.logger.warning("Player is dead!")
                            self.running = False
                            break
                        elif "message" in line:
                            self._handle_message(line)
                        else:
                            self.response_queue.put(line)

            except socket.error as e:
                self.logger.error(f"Socket error in receiver: {e}")
                self.running = False
                break

    def _handle_message(self, message):
        """Handle broadcast messages"""
        # Extract direction and content
        match = re.match(r"message (\d+), (.*)", message)
        if match:
            direction = int(match.group(1))
            content = match.group(2)

            # Debug the raw message
            self.logger.debug(f"Received message from direction {direction}: {content}")

            # Check if the message starts with our team's encrypted format
            if content.startswith("ENC:"):
                try:
                    encrypted_part = content[4:]  # Skip the "ENC:" prefix
                    decrypted_content = decrypt_message(encrypted_part, self.encryption_key)
                    self.logger.info(f"Successfully decrypted team message: {decrypted_content}")

                    # Store the direction immediately for fast response
                    if "TEAM:" in decrypted_content:
                        parts = decrypted_content.split(":")
                        if len(parts) >= 2:
                            sender_id = parts[1]
                            # Store this high-priority direction to move toward
                            self.team_member_positions[sender_id] = {
                                "direction": direction,
                                "timestamp": time.time(),
                                "high_priority": True
                            }

                            # Immediately attempt to move toward the signal if it's an urgent message
                            message_type = parts[2] if len(parts) >= 3 else ""
                            if message_type in ["ELEVATION", "NEED_RESOURCES", "NEED_HELP"]:
                                # Try to move immediately toward the sender
                                self.logger.info(f"Urgent message received! Moving toward {sender_id} in direction {direction}")
                                self._move_toward_player(direction)

                except Exception as e:
                    self.logger.warning(f"Failed to decrypt message: {e}")
                    return
            else:
                # Otherwise, just use the content as is (might be from another team)
                decrypted_content = content
                self.logger.info(f"Processing unencrypted message: {decrypted_content}")

            # Process message based on content
            if decrypted_content.startswith("TEAM:"):
                parts = decrypted_content.split(":")
                if len(parts) >= 3:
                    sender_id = parts[1]
                    message_type = parts[2]

                    self.logger.info(f"Team message from {sender_id}, type: {message_type}")

                    # Update sender's estimated position based on sound direction
                    self.team_member_positions[sender_id] = {
                        "direction": direction,
                        "timestamp": time.time()
                    }

                    if message_type == "ELEVATION":
                        # Someone is asking for elevation
                        level = int(parts[3]) if len(parts) > 3 else 1
                        resources_data = parts[4] if len(parts) > 4 else ""

                        self.logger.info(f"Received elevation request from {sender_id} for level {level}")

                        if level == self.level:
                            self.logger.info(f"Request matches my level ({self.level})")
                            self.incantation_participants[sender_id] = {
                                "type": "ELEVATION",
                                "level": level,
                                "resources": resources_data,
                                "direction": direction,
                                "timestamp": time.time()
                            }

                            # If we're not already in elevation state and have the required resources
                            if self.state != "ELEVATION" and self._has_required_resources_for_level():
                                self.logger.info(f"I have resources and not busy, responding to help {sender_id}")
                                # Send a response that we're available to help
                                self._send_elevation_response(sender_id)
                            else:
                                has_resources = self._has_required_resources_for_level()
                                self.logger.info(f"Cannot help with elevation: busy={self.state=='ELEVATION'}, has_resources={has_resources}")
                        else:
                            self.logger.info(f"Request for different level ({level}), my level is {self.level}")

                    elif message_type == "ELEVATION_RESPONSE":
                        # Someone is responding to our elevation request
                        level = int(parts[3]) if len(parts) > 3 else 1

                        self.logger.info(f"Received elevation response from {sender_id} for level {level}")

                        if level == self.level and self.state == "ELEVATION":
                            # Prevent duplicate entries
                            if sender_id not in self.incantation_participants:
                                self.incantation_participants[sender_id] = {
                                    "type": "ELEVATION_RESPONSE",
                                    "level": level,
                                    "timestamp": time.time()
                                }
                                self.logger.info(f"Added participant {sender_id} to elevation (total: {len(self.incantation_participants)})")
                            else:
                                self.logger.info(f"Participant {sender_id} already in list")
                        else:
                            self.logger.info(f"Cannot use response: myLevel={self.level}, theirLevel={level}, myState={self.state}")

                    elif message_type == "RESOURCES":
                        # Someone is sharing resource information
                        self.incantation_participants[sender_id] = {
                            "type": "RESOURCES",
                            "resources": parts[3] if len(parts) > 3 else "",
                            "timestamp": time.time()
                        }

                    elif message_type == "POSITION":
                        # Someone is sharing their position information
                        x = int(parts[3]) if len(parts) > 3 and parts[3].isdigit() else 0
                        y = int(parts[4]) if len(parts) > 4 and parts[4].isdigit() else 0

                        self.team_member_positions[sender_id] = {
                            "x": x,
                            "y": y,
                            "direction": direction,
                            "timestamp": time.time()
                        }

                    elif message_type == "LEVEL_UP":
                        # Someone just leveled up
                        new_level = int(parts[3]) if len(parts) > 3 and parts[3].isdigit() else 0
                        self.logger.info(f"Team member {sender_id} reached level {new_level}!")

                    elif message_type == "NEED_RESOURCES":
                        # Someone is asking for specific resources
                        resource_type = parts[3] if len(parts) > 3 else ""

                        # Check if we have excess of that resource
                        if resource_type and resource_type in ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]:
                            resource_enum = self._get_resource_enum(resource_type)

                            # If we have more than we need for our next level
                            if resource_enum and self.inventory[resource_enum] > self.target_resources.get(resource_enum, 0) + 1:
                                # Let them know we can share
                                self._send_resource_sharing_offer(sender_id, resource_type)

    def move_forward(self):
        """Move forward one tile"""
        response = self._send_command("Forward")
        return response == "ok"

    def turn_right(self):
        """Turn 90° right"""
        response = self._send_command("Right")
        if response == "ok":
            # Update direction
            if self.direction == Direction.NORTH:
                self.direction = Direction.EAST
            elif self.direction == Direction.EAST:
                self.direction = Direction.SOUTH
            elif self.direction == Direction.SOUTH:
                self.direction = Direction.WEST
            elif self.direction == Direction.WEST:
                self.direction = Direction.NORTH
        return response == "ok"

    def turn_left(self):
        """Turn 90° left"""
        response = self._send_command("Left")
        if response == "ok":
            # Update direction
            if self.direction == Direction.NORTH:
                self.direction = Direction.WEST
            elif self.direction == Direction.WEST:
                self.direction = Direction.SOUTH
            elif self.direction == Direction.SOUTH:
                self.direction = Direction.EAST
            elif self.direction == Direction.EAST:
                self.direction = Direction.NORTH
        return response == "ok"

    def look_around(self):
        """Look around and return what's seen"""
        response = self._send_command("Look")
        if response.startswith("[") and response.endswith("]"):
            # Parse the tiles information
            tiles_str = response[1:-1].split(",")
            tiles = []
            for tile_str in tiles_str:
                tile = []
                items = tile_str.strip().split()
                for item in items:
                    tile.append(item)
                tiles.append(tile)
            return tiles
        return []

    def get_inventory(self):
        """Get the player's inventory"""
        response = self._send_command("Inventory")
        if response.startswith("[") and response.endswith("]"):
            items = response[1:-1].split(",")
            for item in items:
                item = item.strip()
                if item:
                    name, count = item.split()
                    count = int(count)

                    if "food" in name:
                        self.inventory[ResourceType.FOOD] = count
                    elif "linemate" in name:
                        self.inventory[ResourceType.LINEMATE] = count
                    elif "deraumere" in name:
                        self.inventory[ResourceType.DERAUMERE] = count
                    elif "sibur" in name:
                        self.inventory[ResourceType.SIBUR] = count
                    elif "mendiane" in name:
                        self.inventory[ResourceType.MENDIANE] = count
                    elif "phiras" in name:
                        self.inventory[ResourceType.PHIRAS] = count
                    elif "thystame" in name:
                        self.inventory[ResourceType.THYSTAME] = count

        return self.inventory

    def broadcast(self, text):
        """Broadcast a message"""
        encrypted_message = encrypt_message(text, str(self.client_num))
        response = self._send_command(f"Broadcast {encrypted_message}")
        return response == "ok"

    def broadcast_team_message(self, message):
        """Broadcast an encrypted message to team members"""
        # Rate limit broadcasts to avoid overloading
        current_time = time.time()
        if current_time - self.last_broadcast_time < self.broadcast_cooldown:
            self.logger.info(f"Throttled: last broadcast was {current_time - self.last_broadcast_time}s ago")
            return False

        self.logger.info(f"Sending team message: {message}")

        # Encrypt the message with team key
        encrypted = f"ENC:{encrypt_message(message, self.encryption_key)}"
        self.logger.debug(f"Encrypted form: {encrypted}")

        # Send and update timestamp
        result = self.broadcast(encrypted)
        self.logger.info(f"Broadcast result: {result}")

        if result:
            self.last_broadcast_time = current_time

        return result

    def _send_elevation_response(self, requester_id):
        """Send a response to an elevation request"""
        self.logger.info(f"Sending elevation response to {requester_id} for level {self.level}")
        message = f"TEAM:{self.client_num}:ELEVATION_RESPONSE:{self.level}"
        success = self.broadcast_team_message(message)
        self.logger.info(f"Response sent successfully: {success}")

        # If we're not already trying to elevate, change to ELEVATION state
        # This helps AI actively move toward helping with elevation
        if self.state != "ELEVATION" and success:
            self.logger.info(f"Changing state to ELEVATION to help client {requester_id}")
            self.state = "ELEVATION"

    def _send_resource_sharing_offer(self, requester_id, resource_type):
        """Offer to share resources with a team member"""
        message = f"TEAM:{self.client_num}:RESOURCE_OFFER:{resource_type}"
        self.broadcast_team_message(message)

    def _send_position_update(self):
        """Share our position estimate with team members"""
        # We don't know our exact position, but we can share what we think it is
        if self.position and isinstance(self.position, (list, tuple)) and len(self.position) >= 2:
            message = f"TEAM:{self.client_num}:POSITION:{self.position[0]}:{self.position[1]}"
            self.broadcast_team_message(message)
            return True
        return False

    def _broadcast_level_up(self):
        """Announce to the team that we've leveled up"""
        message = f"TEAM:{self.client_num}:LEVEL_UP:{self.level}"
        self.broadcast_team_message(message)

    def _request_resources(self, resource_type):
        """Ask team members if they have excess of a specific resource"""
        message = f"TEAM:{self.client_num}:NEED_RESOURCES:{resource_type}"
        self.broadcast_team_message(message)

    def _get_resource_enum(self, resource_name):
        """Convert string resource name to ResourceType enum"""
        name_to_enum = {
            "food": ResourceType.FOOD,
            "linemate": ResourceType.LINEMATE,
            "deraumere": ResourceType.DERAUMERE,
            "sibur": ResourceType.SIBUR,
            "mendiane": ResourceType.MENDIANE,
            "phiras": ResourceType.PHIRAS,
            "thystame": ResourceType.THYSTAME
        }
        return name_to_enum.get(resource_name.lower())

    def _has_required_resources_for_level(self):
        """Check if we have all the resources needed for the current level"""
        if self.level >= 8:
            self.logger.info("Already at max level")
            return False  # Max level reached

        _, linemate, deraumere, sibur, mendiane, phiras, thystame = self.ELEVATION_REQUIREMENTS[self.level]

        has_linemate = self.inventory[ResourceType.LINEMATE] >= linemate
        has_deraumere = self.inventory[ResourceType.DERAUMERE] >= deraumere
        has_sibur = self.inventory[ResourceType.SIBUR] >= sibur
        has_mendiane = self.inventory[ResourceType.MENDIANE] >= mendiane
        has_phiras = self.inventory[ResourceType.PHIRAS] >= phiras
        has_thystame = self.inventory[ResourceType.THYSTAME] >= thystame

        result = (has_linemate and has_deraumere and has_sibur and
                 has_mendiane and has_phiras and has_thystame)

        self.logger.debug(f"Level {self.level} requirements check:")
        self.logger.debug(f"linemate: {self.inventory[ResourceType.LINEMATE]}/{linemate} - {has_linemate}")
        self.logger.debug(f"deraumere: {self.inventory[ResourceType.DERAUMERE]}/{deraumere} - {has_deraumere}")
        self.logger.debug(f"sibur: {self.inventory[ResourceType.SIBUR]}/{sibur} - {has_sibur}")
        self.logger.debug(f"mendiane: {self.inventory[ResourceType.MENDIANE]}/{mendiane} - {has_mendiane}")
        self.logger.debug(f"phiras: {self.inventory[ResourceType.PHIRAS]}/{phiras} - {has_phiras}")
        self.logger.debug(f"thystame: {self.inventory[ResourceType.THYSTAME]}/{thystame} - {has_thystame}")
        self.logger.info(f"All requirements met: {result}")

        return result

    def connect_nbr(self):
        """Get the number of available slots in the team"""
        response = self._send_command("Connect_nbr")
        try:
            return int(response)
        except ValueError:
            return 0

    def fork(self):
        """Fork a new player"""
        response = self._send_command("Fork")
        return response == "ok"

    def eject(self):
        """Eject players from this tile"""
        response = self._send_command("Eject")
        return response == "ok"

    def take_object(self, object_name):
        """Take an object from the ground"""
        response = self._send_command(f"Take {object_name}")
        return response == "ok"

    def set_object(self, object_name):
        """Place an object on the ground"""
        response = self._send_command(f"Set {object_name}")
        return response == "ok"

    def incantation(self):
        """Start an incantation"""
        self.logger.info("Starting incantation ritual for level " + str(self.level))

        # For levels > 1, double-check that we have enough players
        if self.level > 1:
            tiles = self.look_around()
            if tiles:
                current_tile = self._analyze_tile(tiles[0])
                required_players = self.ELEVATION_REQUIREMENTS[self.level][0]
                total_players = current_tile["players"] + 1  # +1 for self

                if total_players < required_players:
                    self.logger.error(f"Cannot start incantation: not enough players! Have {total_players}, need {required_players}")
                    return False

        # Log current inventory before incantation
        self.logger.info(f"Current inventory before incantation: {self.inventory}")

        response = self._send_command("Incantation")
        self.logger.info(f"Incantation command response: {response}")

        if "ko" in response.lower():
            self.logger.error(f"Incantation failed with response: {response}")
            return False

        if "Elevation underway" in response:
            self.logger.info("Elevation is underway, waiting for result...")
            try:
                # Wait for the result
                result = self.response_queue.get(timeout=10)
                self.logger.info(f"Result received: {result}")

                if "Current level" in result:
                    # Extract the new level
                    match = re.search(r"Current level: (\d+)", result)
                    if match:
                        old_level = self.level
                        self.level = int(match.group(1))
                        self.vision_range = self.level
                        self.logger.info(f"Level up successful: {old_level} -> {self.level}")
                        self._update_target_resources()
                        return True
                    else:
                        self.logger.warning("Failed to extract new level from response")
                else:
                    self.logger.warning("Response does not contain level information")
            except queue.Empty:
                self.logger.warning("Timed out waiting for elevation result")
        else:
            self.logger.warning(f"Elevation was not started. Response: {response}")

        return False

    def _get_resource_name(self, resource_type):
        """Convert ResourceType to string name used by the server"""
        if resource_type == ResourceType.FOOD:
            return "food"
        elif resource_type == ResourceType.LINEMATE:
            return "linemate"
        elif resource_type == ResourceType.DERAUMERE:
            return "deraumere"
        elif resource_type == ResourceType.SIBUR:
            return "sibur"
        elif resource_type == ResourceType.MENDIANE:
            return "mendiane"
        elif resource_type == ResourceType.PHIRAS:
            return "phiras"
        elif resource_type == ResourceType.THYSTAME:
            return "thystame"
        return ""

    def _analyze_tile(self, tile):
        """Analyze the contents of a tile"""
        result = {
            "players": 0,
            "food": 0,
            "linemate": 0,
            "deraumere": 0,
            "sibur": 0,
            "mendiane": 0,
            "phiras": 0,
            "thystame": 0
        }

        for item in tile:
            if item == "player":
                result["players"] += 1
            elif item == "food":
                result["food"] += 1
            elif item == "linemate":
                result["linemate"] += 1
            elif item == "deraumere":
                result["deraumere"] += 1
            elif item == "sibur":
                result["sibur"] += 1
            elif item == "mendiane":
                result["mendiane"] += 1
            elif item == "phiras":
                result["phiras"] += 1
            elif item == "thystame":
                result["thystame"] += 1

        return result

    def _check_elevation_requirements(self):
        """Check if we can perform an elevation ritual"""
        if self.level >= 8:
            self.logger.info("Already at max level")
            return False  # Max level reached

        # Check if we have the required resources in inventory
        required_players, linemate, deraumere, sibur, mendiane, phiras, thystame = self.ELEVATION_REQUIREMENTS[self.level]
        self.logger.info(f"Level {self.level} elevation requires: {required_players} players, L:{linemate}, D:{deraumere}, S:{sibur}, M:{mendiane}, P:{phiras}, T:{thystame}")

        # Check number of players on the same tile (including self)
        tiles = self.look_around()
        if not tiles:
            self.logger.info("No tiles visible")
            return False

        current_tile = tiles[0]  # The first tile is where we are standing
        tile_analysis = self._analyze_tile(current_tile)
        self.logger.info(f"Current tile has {tile_analysis['players']} other players (need {required_players} including self)")

        # +1 for counting ourselves
        if tile_analysis["players"] + 1 < required_players:
            self.logger.info(f"Not enough players: have {tile_analysis['players'] + 1}, need {required_players}")

            # For level 1, we only need ourselves (1 player)
            if self.level == 1 and required_players == 1:
                self.logger.info("Level 1 only requires 1 player (ourselves), proceeding with elevation")
                return True

            # If we have responses from potential participants for elevation
            if len(self.incantation_participants) > 0:
                self.logger.info(f"But {len(self.incantation_participants)} participants have responded, so returning true")
                return True

            return False

        # Check resources in inventory that we need to place
        has_linemate = self.inventory[ResourceType.LINEMATE] >= linemate
        has_deraumere = self.inventory[ResourceType.DERAUMERE] >= deraumere
        has_sibur = self.inventory[ResourceType.SIBUR] >= sibur
        has_mendiane = self.inventory[ResourceType.MENDIANE] >= mendiane
        has_phiras = self.inventory[ResourceType.PHIRAS] >= phiras
        has_thystame = self.inventory[ResourceType.THYSTAME] >= thystame

        all_resources = (has_linemate and has_deraumere and has_sibur and
                         has_mendiane and has_phiras and has_thystame)

        self.logger.info(f"Resources check: L:{has_linemate}, D:{has_deraumere}, S:{has_sibur}, M:{has_mendiane}, P:{has_phiras}, T:{has_thystame}")
        self.logger.info(f"All resources available: {all_resources}")

        return all_resources

    def _check_resources_on_tile(self):
        """Check resources available on the current tile"""
        tiles = self.look_around()
        if not tiles:
            return {}

        current_tile = tiles[0]
        return self._analyze_tile(current_tile)

    def _prepare_for_elevation(self):
        """Place the required resources on the ground for elevation"""
        if self.level >= 8:
            self.logger.info("Already at max level")
            return False

        _, linemate, deraumere, sibur, mendiane, phiras, thystame = self.ELEVATION_REQUIREMENTS[self.level]
        self.logger.info(f"Need to place: linemate={linemate}, deraumere={deraumere}, sibur={sibur}, mendiane={mendiane}, phiras={phiras}, thystame={thystame}")

        # Check what's already on the current tile
        tiles = self.look_around()
        if not tiles or len(tiles) == 0:
            self.logger.warning("Cannot see current tile, cannot proceed with elevation")
            return False

        current_tile = self._analyze_tile(tiles[0])
        self.logger.info(f"Current tile contents before placing resources: {current_tile}")

        # Place the resources on the ground
        resources_placed = 0

        if linemate > 0 and self.inventory[ResourceType.LINEMATE] >= linemate:
            for i in range(linemate):
                self.logger.info(f"Placing linemate {i+1}/{linemate}")
                if self.set_object("linemate"):
                    resources_placed += 1
                    self.logger.info("Linemate placed successfully")
                else:
                    self.logger.warning("Failed to place linemate")

        if deraumere > 0 and self.inventory[ResourceType.DERAUMERE] >= deraumere:
            for i in range(deraumere):
                self.logger.info(f"Placing deraumere {i+1}/{deraumere}")
                if self.set_object("deraumere"):
                    resources_placed += 1
                    self.logger.info("Deraumere placed successfully")
                else:
                    self.logger.warning("Failed to place deraumere")

        if sibur > 0 and self.inventory[ResourceType.SIBUR] >= sibur:
            for i in range(sibur):
                self.logger.info(f"Placing sibur {i+1}/{sibur}")
                if self.set_object("sibur"):
                    resources_placed += 1
                    self.logger.info("Sibur placed successfully")
                else:
                    self.logger.warning("Failed to place sibur")

        if mendiane > 0 and self.inventory[ResourceType.MENDIANE] >= mendiane:
            for i in range(mendiane):
                self.logger.info(f"Placing mendiane {i+1}/{mendiane}")
                if self.set_object("mendiane"):
                    resources_placed += 1
                    self.logger.info("Mendiane placed successfully")
                else:
                    self.logger.warning("Failed to place mendiane")

        if phiras > 0 and self.inventory[ResourceType.PHIRAS] >= phiras:
            for i in range(phiras):
                self.logger.info(f"Placing phiras {i+1}/{phiras}")
                if self.set_object("phiras"):
                    resources_placed += 1
                    self.logger.info("Phiras placed successfully")
                else:
                    self.logger.warning("Failed to place phiras")

        if thystame > 0 and self.inventory[ResourceType.THYSTAME] >= thystame:
            for i in range(thystame):
                self.logger.info(f"Placing thystame {i+1}/{thystame}")
                if self.set_object("thystame"):
                    resources_placed += 1
                    self.logger.info("Thystame placed successfully")
                else:
                    self.logger.warning("Failed to place thystame")

        # Check if we placed all required resources
        expected_resources = linemate + deraumere + sibur + mendiane + phiras + thystame
        success = resources_placed == expected_resources
        self.logger.info(f"Resources placed: {resources_placed}/{expected_resources}, success={success}")
        return success

    def _collect_resources_from_tile(self):
        """Collect resources from the current tile based on needs"""
        tile_contents = self._check_resources_on_tile()
        resources_collected = 0

        # Always collect food first
        if tile_contents.get("food", 0) > 0:
            if self.take_object("food"):
                resources_collected += 1
                return True

        # Collect other resources based on elevation requirements
        for resource_type, amount_needed in self.target_resources.items():
            resource_name = self._get_resource_name(resource_type)
            # If we need more of this resource and it's on the tile
            if (self.inventory[resource_type] < amount_needed and
                tile_contents.get(resource_name, 0) > 0):
                if self.take_object(resource_name):
                    resources_collected += 1
                    return True

        return resources_collected > 0

    def _decide_next_action(self):
        """Decide what to do next based on current state"""
        # First, always check our inventory
        old_state = self.state
        self.get_inventory()

        # Adjust food threshold based on level - higher levels need more food priority
        food_threshold = 5
        if self.level >= 2:
            food_threshold = 10  # More aggressive food searching at higher levels

        # If food is critically low, prioritize finding food
        if self.inventory[ResourceType.FOOD] <= food_threshold:
            if self.state != "SEARCHING_FOOD":
                self.logger.info(f"Changing state from {self.state} to SEARCHING_FOOD (food={self.inventory[ResourceType.FOOD]})")
                self.state = "SEARCHING_FOOD"
            return

        # Check if we can perform an elevation ritual
        elevation_possible = self._check_elevation_requirements()
        self.logger.info(f"Elevation requirements check: {elevation_possible}")

        if elevation_possible:
            # Try to initiate elevation if we have all requirements
            if self.state != "ELEVATION":
                self.logger.info(f"Changing state from {self.state} to ELEVATION (level {self.level})")
                self.state = "ELEVATION"
                # Initialize elevation_attempt_time when changing to ELEVATION state
                self.elevation_attempt_time = time.time()
            return

        # Decide what resources we need to collect
        missing_resources = False
        for resource_type, amount_needed in self.target_resources.items():
            if self.inventory[resource_type] < amount_needed:
                self.logger.info(f"Missing resource: {self._get_resource_name(resource_type)} ({self.inventory[resource_type]}/{amount_needed})")
                missing_resources = True
                break

        if missing_resources:
            if self.state != "COLLECTING_RESOURCES":
                self.logger.info(f"Changing state from {self.state} to COLLECTING_RESOURCES")
                self.state = "COLLECTING_RESOURCES"
            return

        # By default, continue searching for resources
        if self.state != "SEARCHING_RESOURCES":
            self.logger.info(f"Changing state from {self.state} to SEARCHING_RESOURCES")
            self.state = "SEARCHING_RESOURCES"

    def _perform_random_movement(self):
        """Perform a random movement to explore the map"""
        # Random choice: move forward or change direction
        choice = random.randint(0, 3)

        if choice == 0:  # Move forward
            return self.move_forward()
        elif choice == 1:  # Turn right
            return self.turn_right()
        elif choice == 2:  # Turn left
            return self.turn_left()
        else:  # Look around
            self.look_around()
            return True

    def _broadcast_status(self):
        """Broadcast our status to teammates"""
        current_time = time.time()
        if current_time - self.last_broadcast_time < self.broadcast_cooldown:
            return  # Respect cooldown

        # Format: "TEAM:ID:TYPE:DATA"
        if self.state == "ELEVATION" and self.level > 1:
            # Share what resources we have for elevation (only for levels > 1)
            resources = ",".join([f"{self._get_resource_name(r)}:{self.inventory[r]}"
                               for r in [ResourceType.LINEMATE, ResourceType.DERAUMERE,
                                        ResourceType.SIBUR, ResourceType.MENDIANE,
                                        ResourceType.PHIRAS, ResourceType.THYSTAME]])
            message = f"TEAM:{self.client_num}:ELEVATION:{self.level}:{resources}"
            self.broadcast_team_message(message)

        elif random.random() < 0.1:  # Occasionally share other information
            if self.state == "COLLECTING_RESOURCES":
                # Share our inventory with teammates
                resources = ",".join([f"{self._get_resource_name(r)}:{self.inventory[r]}"
                                   for r in self.inventory
                                   if r != ResourceType.FOOD])
                message = f"TEAM:{self.client_num}:RESOURCES:{resources}"
                self.broadcast_team_message(message)

            # Periodically share position estimates
            if random.random() < 0.2:
                self._send_position_update()

    def _handle_food_search(self):
        """Search for food when hungry"""
        # Look around for food
        tiles = self.look_around()

        # Broadcast for help if food is critically low (especially for level 2+)
        if self.inventory[ResourceType.FOOD] <= 3 or (self.level >= 2 and self.inventory[ResourceType.FOOD] <= 5):
            # Ask for help more urgently
            self._broadcast_need_help()

        if tiles:
            # Check if there's food on current tile
            current_tile = self._analyze_tile(tiles[0])
            if current_tile["food"] > 0:
                if self.take_object("food"):
                    self.logger.info(f"Found and took food! Current food: {self.inventory[ResourceType.FOOD]}")
                    return True

            # Check nearby tiles for food
            for i in range(1, len(tiles)):
                tile = self._analyze_tile(tiles[i])
                if tile["food"] > 0:
                    # Move toward food
                    self.logger.info(f"Found food on tile {i}, moving toward it")
                    self.move_forward()
                    return True

        # Check if any teammates have signaled recently - maybe they have food
        for sender_id, pos_data in self.team_member_positions.items():
            if 'direction' in pos_data and time.time() - pos_data.get('timestamp', 0) < 15:  # Messages less than 15 seconds old
                if pos_data.get('high_priority', False):
                    # Move toward the teammate who signaled
                    self.logger.info(f"Moving toward teammate {sender_id} in direction {pos_data['direction']}")
                    return self._move_toward_player(pos_data['direction'])

        # No food visible, move randomly
        return self._perform_random_movement()

    def _handle_resource_collection(self):
        """Collect resources based on needs"""
        # Try to collect resources from current tile
        if self._collect_resources_from_tile():
            return True

        # Look around for resources
        tiles = self.look_around()
        if not tiles:
            return self._perform_random_movement()

        # Check nearby tiles for needed resources
        found_resource = False
        for i in range(1, len(tiles)):
            tile = self._analyze_tile(tiles[i])
            for resource_type, amount_needed in self.target_resources.items():
                resource_name = self._get_resource_name(resource_type)
                if self.inventory[resource_type] < amount_needed and tile[resource_name] > 0:
                    found_resource = True
                    break
            if found_resource:
                break

        if found_resource:
            # Move toward resource (simplistic approach: just move forward)
            self.move_forward()
            return True
        else:
            # No visible needed resources, move randomly
            return self._perform_random_movement()

    def _handle_elevation(self):
        """Handle the elevation process"""
        # First, check how many players are on our tile
        tiles = self.look_around()
        if not tiles:
            self.logger.info("No tiles visible, can't proceed")
            self.state = "COLLECTING_RESOURCES"
            return False

        current_tile = self._analyze_tile(tiles[0])
        required_players = self.ELEVATION_REQUIREMENTS[self.level][0]
        total_players = current_tile['players'] + 1  # +1 for self

        # Debug info about current situation
        self.logger.info(f"Client {self.client_num}, Level {self.level}: {current_tile['players']} other players detected on my tile (total: {total_players}, need {required_players})")

        # If not enough players present for levels > 1, abort elevation immediately
        if self.level > 1 and total_players < required_players:
            self.logger.warning(f"Not enough players for elevation! Have {total_players}, need {required_players}")
            self.state = "COLLECTING_RESOURCES"
            return False

        # More detailed inventory logging
        self.logger.info(f"Current inventory: {[(self._get_resource_name(r), self.inventory[r]) for r in [ResourceType.LINEMATE, ResourceType.DERAUMERE, ResourceType.SIBUR, ResourceType.MENDIANE, ResourceType.PHIRAS, ResourceType.THYSTAME]]}")
        self.logger.info(f"Resources needed: {self.ELEVATION_REQUIREMENTS[self.level][1:]}")

        # Special case for level 1 - no need for other players
        if self.level == 1:
            self.logger.info("Level 1 elevation attempt - requires only ourselves")

            # Check if we have the required linemate
            if self.inventory[ResourceType.LINEMATE] >= 1:
                # Place resources for level 1
                resources_placed = self._prepare_for_elevation()
                self.logger.info(f"Resources placed for level 1: {resources_placed}")

                if resources_placed:
                    # Start incantation
                    self.logger.info("Starting level 1 incantation...")
                    result = self.incantation()
                    if result:
                        self.logger.info(f"Level 1 elevation successful! New level: {self.level}")
                        self.state = "COLLECTING_RESOURCES"
                        return True
                    else:
                        self.logger.warning("Level 1 elevation failed!")
                        self.state = "COLLECTING_RESOURCES"
                        return False
                else:
                    self.logger.warning("Failed to place resources for level 1")
                    self.state = "COLLECTING_RESOURCES"
                    return False
            else:
                self.logger.warning("Don't have the required linemate for level 1!")
                self.state = "COLLECTING_RESOURCES"
                return False

        # For levels > 1, handle team coordination
        else:
            # If not enough players, broadcast for help
            if current_tile["players"] + 1 < required_players:
                # Prepare a detailed resource inventory to share
                resources = ",".join([f"{self._get_resource_name(r)}:{self.inventory[r]}"
                                  for r in [ResourceType.LINEMATE, ResourceType.DERAUMERE,
                                            ResourceType.SIBUR, ResourceType.MENDIANE,
                                            ResourceType.PHIRAS, ResourceType.THYSTAME]])

                # Send encrypted message to team members
                message = f"TEAM:{self.client_num}:ELEVATION:{self.level}:{resources}"

                # Debug message before broadcasting
                self.logger.info(f"Broadcasting for help: {message}")
                success = self.broadcast_team_message(message)
                self.logger.info(f"Broadcast success: {success}")

                # Check if we have teammates who have responded
                if len(self.incantation_participants) > 0:
                    # Wait a bit longer for them to arrive
                    self.logger.info(f"Waiting for {len(self.incantation_participants)} teammates to arrive (IDs: {self.incantation_participants})")
                    time.sleep(1)  # Brief pause to wait
                else:
                    # No responses yet, keep collecting or retry later
                    self.logger.info("No responses from teammates, returning to resource collection")
                    self.state = "COLLECTING_RESOURCES"
                    return False

            # Check if we have enough players now
            tiles = self.look_around()
            if tiles:
                current_tile = self._analyze_tile(tiles[0])
                self.logger.info(f"Rechecking tile: {current_tile['players']} players visible (need {required_players})")

                # We need to check if there are actually enough players on the tile
                total_players = current_tile["players"] + 1  # +1 for self
                if total_players >= required_players:
                    self.logger.info(f"Enough players detected ({total_players}/{required_players}), preparing for elevation")

                    # Place resources for elevation
                    resources_placed = self._prepare_for_elevation()
                    self.logger.info(f"Resources placed: {resources_placed}")

                    if resources_placed:
                        # Double-check player count before starting incantation
                        tiles = self.look_around()
                        if tiles:
                            current_tile = self._analyze_tile(tiles[0])
                            total_players = current_tile["players"] + 1  # +1 for self

                            if total_players < required_players:
                                self.logger.warning(f"Not enough players! Only have {total_players}/{required_players} - cancelling incantation")
                                self.state = "COLLECTING_RESOURCES"
                                return False

                        # Start incantation
                        self.logger.info(f"Starting incantation with {total_players} players (required: {required_players})...")
                        result = self.incantation()
                        if result:
                            self.logger.info(f"Elevation successful! New level: {self.level}")
                            # Announce our level up to teammates
                            self._broadcast_level_up()
                            # Reset participants list
                            self.incantation_participants = []
                            self.state = "COLLECTING_RESOURCES"
                            return True
                        else:
                            self.logger.warning("Elevation failed! Checking resources on tile:")
                            # Debug what's on the current tile
                            if tiles and len(tiles) > 0:
                                self.logger.info(f"Current tile contents: {tiles[0]}")
                            self.incantation_participants = []  # Reset
                            self.state = "COLLECTING_RESOURCES"
                            return False
                    else:
                        self.logger.warning("Failed to place all required resources")
                        self.state = "COLLECTING_RESOURCES"
                        return False
                else:
                    self.logger.info(f"Not enough players yet ({current_tile['players'] + 1}/{required_players})")
                    self.state = "COLLECTING_RESOURCES"
                    return False

        # If we reach here, something went wrong
        self.state = "COLLECTING_RESOURCES"
        return False

    def _broadcast_need_help(self):
        """Broadcast that we need help (low food or critical resources)"""
        message = f"TEAM:{self.client_num}:NEED_HELP:{self.level}:{self.inventory[ResourceType.FOOD]}"
        self.broadcast_team_message(message)
        self.logger.info("Broadcasted help request due to critical situation")
        return True

    def run(self):
        """Main AI loop"""
        last_fork_time = 0
        fork_cooldown = 100  # Time between fork attempts
        last_resource_request_time = 0
        resource_request_cooldown = 30  # Time between resource requests

        # Reduced cooldown for broadcasts at level 2+ to improve communication
        self.broadcast_cooldown = 1 if self.level >= 2 else 2

        try:
            while self.running:
                # First, always check inventory
                self.get_inventory()

                # Decide what to do next
                self._decide_next_action()

                # Check for team signals that need immediate attention
                current_time = time.time()
                urgent_signal = False

                for sender_id, pos_data in self.team_member_positions.items():
                    if 'high_priority' in pos_data and pos_data['high_priority'] and \
                       current_time - pos_data.get('timestamp', 0) < 10:  # Recent urgent message
                        self.logger.info(f"Responding to urgent signal from {sender_id}")
                        self._move_toward_player(pos_data['direction'])
                        urgent_signal = True
                        break

                # Only continue with normal behavior if not responding to urgent signal
                if not urgent_signal:
                    # Broadcast status to teammates as needed
                    self._broadcast_status()

                    # For level 2+, be more aggressive about requesting help when food is low
                    if self.level >= 2 and self.inventory[ResourceType.FOOD] <= 7:
                        self._broadcast_need_help()

                    # Check if we need specific resources for elevation
                    if (self.state == "COLLECTING_RESOURCES" and
                        current_time - last_resource_request_time > resource_request_cooldown):

                        # Find what resources we're missing for next level
                        for res_type, amount_needed in self.target_resources.items():
                            if self.inventory[res_type] < amount_needed:
                                # Request this specific resource
                                self._request_resources(self._get_resource_name(res_type))
                                last_resource_request_time = current_time
                                break

                    # Try to fork new players periodically when we have food
                    # Level 2+ AIs need more food before forking
                    food_for_fork = 20 if self.level == 1 else 25
                    if (current_time - last_fork_time > fork_cooldown and
                        self.inventory[ResourceType.FOOD] > food_for_fork):
                        slots = self.connect_nbr()
                        if slots > 0:
                            self.fork()
                            last_fork_time = current_time

                    # Execute action based on current state
                    if self.state == "SEARCHING_FOOD":
                        self._handle_food_search()

                    elif self.state == "COLLECTING_RESOURCES" or self.state == "SEARCHING_RESOURCES":
                        self._handle_resource_collection()

                    elif self.state == "ELEVATION":
                        # Check if we've been trying to elevate for too long
                        if current_time - self.elevation_attempt_time > self.elevation_timeout:
                            self.logger.info("Elevation timed out, returning to resource collection")
                            self.state = "COLLECTING_RESOURCES"
                            self.incantation_participants = {}  # Reset with empty dict
                        else:
                            # If we have a response from another player who needs help with elevation
                            # Try to move toward them
                            moved = False
                            for sender_id, pos_data in self.team_member_positions.items():
                                if 'direction' in pos_data and current_time - pos_data.get('timestamp', 0) < 10:
                                    # Only move toward players who have communicated recently
                                    if sender_id in self.incantation_participants and isinstance(self.incantation_participants, dict) and \
                                       self.incantation_participants[sender_id].get('type') == 'ELEVATION':
                                        self.logger.info(f"Moving toward player {sender_id} for elevation")
                                        self._move_toward_player(pos_data['direction'])
                                        moved = True
                                        break

                            # Try elevation regardless of movement
                            if self._handle_elevation():
                                # Successfully elevated, reset timer
                                self.elevation_attempt_time = 0
                            elif self.elevation_attempt_time == 0:
                                # First attempt at elevation, start timer
                                self.elevation_attempt_time = current_time

                            # If we didn't move toward anyone but we're still in elevation state,
                            # broadcast our status again
                            if not moved and random.random() < 0.3:  # 30% chance each cycle (increased from original)
                                self._broadcast_status()

                # Sleep to avoid overwhelming the server (shorter time for level 2+ for more responsiveness)
                time.sleep(0.08 if self.level >= 2 else 0.1)

        except Exception as e:
            self.logger.error(f"Error in AI loop: {e}")
        finally:
            self.running = False

    def _move_toward_player(self, direction):
        """
        Move toward a player based on sound direction
        direction: the tile number where sound came from
        """
        self.logger.info(f"Trying to move toward player in direction {direction}")

        # Based on the sound direction model, try to move toward that direction
        if direction == 0:  # Same tile
            self.logger.info("Already on the same tile")
            return True
        elif direction == 1:  # Tile in front
            return self.move_forward()
        elif direction == 2:  # 2nd tile in front
            if self.move_forward():
                return self.move_forward()
        elif direction == 3:  # Front-right
            if self.turn_right():
                return self.move_forward()
        elif direction == 4:  # Front-right-right
            if self.turn_right() and self.move_forward():
                self.turn_right()
                return self.move_forward()
        elif direction == 5:  # Right
            if self.turn_right() and self.turn_right():
                return self.move_forward()
        elif direction == 6:  # Back-right
            if self.turn_right() and self.turn_right() and self.turn_right():
                return self.move_forward()
        elif direction == 7:  # Back
            if self.turn_right() and self.turn_right():
                if self.move_forward():
                    return self.move_forward()
        elif direction == 8:  # Back-left
            if self.turn_left() and self.turn_left() and self.turn_left():
                return self.move_forward()

        # If we can't determine a direction or move failed, try a random move
        self.logger.info("Couldn't move in specific direction, trying random movement")
        return self._perform_random_movement()
