#!/usr/bin/env python3

"""
Actions module for the Zappy AI
"""

import re
from models import Direction, ResourceType, get_resource_name

class ActionManager:
    """Manages actions for the Zappy AI"""

    def __init__(self, comm_manager, logger):
        """Initialize the action manager"""
        self.comm_manager = comm_manager
        self.logger = logger
        self.direction = Direction.NORTH  # Default direction

    # Basic movement actions
    def move_forward(self):
        """Move forward one tile"""
        response = self.comm_manager.send_command("Forward")
        return response == "ok"

    def turn_right(self):
        """Turn 90° right"""
        response = self.comm_manager.send_command("Right")
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
        response = self.comm_manager.send_command("Left")
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

    # Perception actions
    def look_around(self):
        """Look around and return what's seen"""
        response = self.comm_manager.send_command("Look")
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

            # Update the vision data in communication manager for terminal UI
            if hasattr(self.comm_manager, 'update_vision_data'):
                # Get vision level from the number of tiles
                # For a level n vision, there are n^2 tiles
                # Level 1: 1 tiles (just current position)
                # Level 2: 4 tiles (current position + 3 adjacent)
                # Level 3: 9 tiles (current position + 8 around)
                # Level n: n^2 tiles

                vision_level = 1
                if len(tiles) > 1:
                    # Calculate vision level based on tiles count
                    tile_count = len(tiles)
                    import math
                    vision_level = int(math.sqrt(tile_count))

                self.comm_manager.update_vision_data(tiles, vision_level)

            return tiles
        return []

    def get_inventory(self):
        """Get the player's inventory"""
        inventory = {
            ResourceType.FOOD: 0,
            ResourceType.LINEMATE: 0,
            ResourceType.DERAUMERE: 0,
            ResourceType.SIBUR: 0,
            ResourceType.MENDIANE: 0,
            ResourceType.PHIRAS: 0,
            ResourceType.THYSTAME: 0
        }

        response = self.comm_manager.send_command("Inventory")
        if response.startswith("[") and response.endswith("]"):
            items = response[1:-1].split(",")
            for item in items:
                item = item.strip()
                if item:
                    name, count = item.split()
                    count = int(count)

                    if "food" in name:
                        inventory[ResourceType.FOOD] = count
                    elif "linemate" in name:
                        inventory[ResourceType.LINEMATE] = count
                    elif "deraumere" in name:
                        inventory[ResourceType.DERAUMERE] = count
                    elif "sibur" in name:
                        inventory[ResourceType.SIBUR] = count
                    elif "mendiane" in name:
                        inventory[ResourceType.MENDIANE] = count
                    elif "phiras" in name:
                        inventory[ResourceType.PHIRAS] = count
                    elif "thystame" in name:
                        inventory[ResourceType.THYSTAME] = count

        return inventory

    # Object interaction
    def take_object(self, object_name):
        """Take an object from the ground"""
        response = self.comm_manager.send_command(f"Take {object_name}")
        return response == "ok"

    def set_object(self, object_name):
        """Place an object on the ground"""
        response = self.comm_manager.send_command(f"Set {object_name}")
        return response == "ok"

    # Team actions
    def connect_nbr(self):
        """Get the number of available slots in the team"""
        response = self.comm_manager.send_command("Connect_nbr")
        try:
            return int(response)
        except ValueError:
            return 0

    def fork(self):
        """Fork a new player"""
        response = self.comm_manager.send_command("Fork")
        return response == "ok"

    def eject(self):
        """Eject players from this tile"""
        response = self.comm_manager.send_command("Eject")
        return response == "ok"

    def incantation(self):
        """Start an incantation"""
        response = self.comm_manager.send_command("Incantation")
        if "Elevation underway" in response:
            # Wait for the result
            result = self.comm_manager.response_queue.get(timeout=10)
            if "Current level" in result:
                # Extract the new level
                match = re.search(r"Current level: (\d+)", result)
                if match:
                    new_level = int(match.group(1))
                    self.logger.info(f"Elevation successful! New level: {new_level}")
                    return new_level
        return 0

    # Analysis methods
    def analyze_tile(self, tile):
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

    def check_resources_on_tile(self):
        """Check resources available on the current tile"""
        tiles = self.look_around()
        if not tiles:
            return {}

        current_tile = tiles[0]
        return self.analyze_tile(current_tile)

    def get_direction(self):
        """Get the current direction"""
        return self.direction
