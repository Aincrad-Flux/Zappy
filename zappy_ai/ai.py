#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

    def __init__(self, socket, client_num, width, height):
        """Initialize the AI"""
        self.socket = socket
        self.client_num = client_num
        self.width = width
        self.height = height
        self.level = 1
        self.inventory = {
            ResourceType.FOOD: 0,
            ResourceType.LINEMATE: 0,
            ResourceType.DERAUMERE: 0,
            ResourceType.SIBUR: 0,
            ResourceType.MENDIANE: 0,
            ResourceType.PHIRAS: 0,
            ResourceType.THYSTAME: 0
        }
        self.position = (0, 0)  # Unknown initially
        self.direction = Direction.NORTH  # Assume starting direction
        self.vision_range = 1  # Initial vision range
        self.message_queue = queue.Queue()
        self.running = True
        self.response_queue = queue.Queue()
        self.state = "SEARCHING_FOOD"  # Initial state
        self.target_resources = {}  # Resources needed for elevation
        self.team_members_nearby = 0
        self.team_messages = {}  # Store messages from team members
        self.incantation_participants = []

        # Calculate resource requirements for next level
        self._update_target_resources()

        # Start the receiver thread
        self.receiver_thread = threading.Thread(target=self._response_receiver)
        self.receiver_thread.daemon = True
        self.receiver_thread.start()

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
            print(f"Error sending command: {e}")
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
                            print("Player is dead!")
                            self.running = False
                            break
                        elif "message" in line:
                            self._handle_message(line)
                        else:
                            self.response_queue.put(line)

            except socket.error as e:
                print(f"Socket error in receiver: {e}")
                self.running = False
                break

    def _handle_message(self, message):
        """Handle broadcast messages"""
        # Extract direction and content
        match = re.match(r"message (\d+), (.*)", message)
        if match:
            direction = int(match.group(1))
            content = match.group(2)

            # Process message based on content
            if content.startswith("TEAM:"):
                parts = content.split(":")
                if len(parts) >= 3:
                    sender_id = parts[1]
                    message_type = parts[2]

                    if message_type == "ELEVATION":
                        # Someone is asking for elevation
                        level = int(parts[3]) if len(parts) > 3 else 1
                        position = parts[4] if len(parts) > 4 else "unknown"

                        if level == self.level:
                            self.team_messages[sender_id] = {
                                "type": "ELEVATION",
                                "level": level,
                                "position": position,
                                "direction": direction,
                                "timestamp": time.time()
                            }
                    elif message_type == "RESOURCES":
                        # Someone is sharing resource information
                        self.team_messages[sender_id] = {
                            "type": "RESOURCES",
                            "resources": parts[3] if len(parts) > 3 else "",
                            "timestamp": time.time()
                        }

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
        response = self._send_command(f"Broadcast {text}")
        return response == "ok"

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
        response = self._send_command("Incantation")
        if "Elevation underway" in response:
            # Wait for the result
            result = self.response_queue.get(timeout=10)
            if "Current level" in result:
                # Extract the new level
                match = re.search(r"Current level: (\d+)", result)
                if match:
                    self.level = int(match.group(1))
                    self.vision_range = self.level
                    self._update_target_resources()
                    return True
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
            return False  # Max level reached

        # Check if we have the required resources in inventory
        required_players, linemate, deraumere, sibur, mendiane, phiras, thystame = self.ELEVATION_REQUIREMENTS[self.level]

        # Check number of players on the same tile (including self)
        tiles = self.look_around()
        if not tiles:
            return False

        current_tile = tiles[0]  # The first tile is where we are standing
        tile_analysis = self._analyze_tile(current_tile)

        # +1 for counting ourselves
        if tile_analysis["players"] + 1 < required_players:
            return False

        # Check resources in inventory that we need to place
        if (self.inventory[ResourceType.LINEMATE] >= linemate and
            self.inventory[ResourceType.DERAUMERE] >= deraumere and
            self.inventory[ResourceType.SIBUR] >= sibur and
            self.inventory[ResourceType.MENDIANE] >= mendiane and
            self.inventory[ResourceType.PHIRAS] >= phiras and
            self.inventory[ResourceType.THYSTAME] >= thystame):
            return True

        return False

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
            return False

        _, linemate, deraumere, sibur, mendiane, phiras, thystame = self.ELEVATION_REQUIREMENTS[self.level]

        # Place the resources on the ground
        resources_placed = 0

        if linemate > 0 and self.inventory[ResourceType.LINEMATE] >= linemate:
            for _ in range(linemate):
                if self.set_object("linemate"):
                    resources_placed += 1

        if deraumere > 0 and self.inventory[ResourceType.DERAUMERE] >= deraumere:
            for _ in range(deraumere):
                if self.set_object("deraumere"):
                    resources_placed += 1

        if sibur > 0 and self.inventory[ResourceType.SIBUR] >= sibur:
            for _ in range(sibur):
                if self.set_object("sibur"):
                    resources_placed += 1

        if mendiane > 0 and self.inventory[ResourceType.MENDIANE] >= mendiane:
            for _ in range(mendiane):
                if self.set_object("mendiane"):
                    resources_placed += 1

        if phiras > 0 and self.inventory[ResourceType.PHIRAS] >= phiras:
            for _ in range(phiras):
                if self.set_object("phiras"):
                    resources_placed += 1

        if thystame > 0 and self.inventory[ResourceType.THYSTAME] >= thystame:
            for _ in range(thystame):
                if self.set_object("thystame"):
                    resources_placed += 1

        # Check if we placed all required resources
        expected_resources = linemate + deraumere + sibur + mendiane + phiras + thystame
        return resources_placed == expected_resources

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
        # First, always check our inventory to know what we have
        self.get_inventory()

        # If food is critically low, prioritize finding food
        if self.inventory[ResourceType.FOOD] <= 5:
            self.state = "SEARCHING_FOOD"
            return

        # Check if we can perform an elevation ritual
        if self._check_elevation_requirements():
            # Try to initiate elevation if we have all requirements
            self.state = "ELEVATION"
            return

        # Decide what resources we need to collect
        missing_resources = False
        for resource_type, amount_needed in self.target_resources.items():
            if self.inventory[resource_type] < amount_needed:
                missing_resources = True
                break

        if missing_resources:
            self.state = "COLLECTING_RESOURCES"
            return

        # By default, continue searching for resources
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
        if random.random() < 0.1:  # Limit broadcasting frequency
            # Format: "TEAM:ID:TYPE:DATA"
            if self.state == "ELEVATION":
                message = f"TEAM:{self.client_num}:ELEVATION:{self.level}:unknown"
                self.broadcast(message)
            elif self.state == "COLLECTING_RESOURCES":
                resources = ",".join([f"{self._get_resource_name(r)}:{self.inventory[r]}"
                                    for r in self.inventory
                                    if r != ResourceType.FOOD])
                message = f"TEAM:{self.client_num}:RESOURCES:{resources}"
                self.broadcast(message)

    def _handle_food_search(self):
        """Search for food when hungry"""
        # Look around for food
        tiles = self.look_around()

        if tiles:
            # Check if there's food on current tile
            current_tile = self._analyze_tile(tiles[0])
            if current_tile["food"] > 0:
                if self.take_object("food"):
                    return True

            # Check nearby tiles for food
            for i in range(1, len(tiles)):
                tile = self._analyze_tile(tiles[i])
                if tile["food"] > 0:
                    # Move toward food (simplistic approach: just move forward)
                    self.move_forward()
                    return True

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
            self.state = "COLLECTING_RESOURCES"
            return False

        current_tile = self._analyze_tile(tiles[0])
        required_players = self.ELEVATION_REQUIREMENTS[self.level][0]

        # If not enough players, broadcast for help
        if current_tile["players"] + 1 < required_players:
            self.broadcast(f"TEAM:{self.client_num}:ELEVATION:{self.level}:unknown")
            # Wait for others or move on after timeout
            self.state = "COLLECTING_RESOURCES"
            return False

        # Place resources for elevation
        if self._prepare_for_elevation():
            # Start incantation
            result = self.incantation()
            if result:
                print(f"Elevation successful! New level: {self.level}")
                self.state = "COLLECTING_RESOURCES"
                return True
            else:
                print("Elevation failed!")
                self.state = "COLLECTING_RESOURCES"
                return False
        else:
            self.state = "COLLECTING_RESOURCES"
            return False

    def run(self):
        """Main AI loop"""
        last_fork_time = 0
        fork_cooldown = 100  # Time between fork attempts

        try:
            while self.running:
                # Decide what to do next
                self._decide_next_action()

                # Broadcast status to teammates occasionally
                self._broadcast_status()

                # Try to fork new players periodically
                current_time = time.time()
                if (current_time - last_fork_time > fork_cooldown and
                    self.inventory[ResourceType.FOOD] > 20):  # Only fork if we have enough food
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
                    self._handle_elevation()

                # Sleep to avoid overwhelming the server
                time.sleep(0.1)

        except Exception as e:
            print(f"Error in AI loop: {e}")
        finally:
            self.running = False
