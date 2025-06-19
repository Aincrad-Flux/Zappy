#!/usr/bin/env python3

"""
Strategy module for the Zappy AI
"""

import random
import time
from models import ResourceType, Direction, ELEVATION_REQUIREMENTS, get_resource_name

class StrategyManager:
    """Manages strategies for the Zappy AI"""

    def __init__(self, action_manager, comm_manager, logger):
        """Initialize the strategy manager"""
        self.action_manager = action_manager
        self.comm_manager = comm_manager
        self.logger = logger
        self.level = 1
        self.vision_range = 1  # Increases with level
        self.state = "COLLECTING_RESOURCES"

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

        # Elevation data
        self.elevation_attempt_time = 0
        self.elevation_timeout = 20  # How long to wait for team members before giving up

    def _update_target_resources(self):
        """Update the target resources needed for the next level"""
        if self.level < 8:
            _, linemate, deraumere, sibur, mendiane, phiras, thystame = ELEVATION_REQUIREMENTS[self.level]
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

    def decide_next_action(self):
        """Decide what to do next based on current state"""
        # First, always update our inventory
        self.inventory = self.action_manager.get_inventory()
        self.logger.debug(f"Current inventory: {self.inventory}")

        # If food is critically low, prioritize finding food
        if self.inventory[ResourceType.FOOD] <= 5:
            self.state = "SEARCHING_FOOD"
            self.logger.info("Food critically low, switching to SEARCHING_FOOD state")
            return

        # Check if we can perform an elevation ritual
        if self._check_elevation_requirements():
            # Try to initiate elevation if we have all requirements
            self.state = "ELEVATION"
            self.logger.info("All elevation requirements met, switching to ELEVATION state")
            return

        # Decide what resources we need to collect
        missing_resources = False
        for resource_type, amount_needed in self.target_resources.items():
            if self.inventory[resource_type] < amount_needed:
                missing_resources = True
                break

        if missing_resources:
            self.state = "COLLECTING_RESOURCES"
            self.logger.info("Missing resources for elevation, switching to COLLECTING_RESOURCES state")
            return

        # By default, continue searching for resources
        self.state = "SEARCHING_RESOURCES"
        self.logger.info("No specific goal, switching to SEARCHING_RESOURCES state")

    def execute_current_state(self):
        """Execute action based on the current state"""
        if self.state == "SEARCHING_FOOD":
            self.logger.debug("Executing SEARCHING_FOOD strategy")
            return self._handle_food_search()
        elif self.state == "COLLECTING_RESOURCES" or self.state == "SEARCHING_RESOURCES":
            self.logger.debug("Executing COLLECTING_RESOURCES strategy")
            return self._handle_resource_collection()
        elif self.state == "ELEVATION":
            self.logger.debug("Executing ELEVATION strategy")
            return self._handle_elevation()

        return False

    def _handle_food_search(self):
        """Search for food when hungry"""
        # Look around for food
        tiles = self.action_manager.look_around()

        if tiles:
            # Check if there's food on current tile
            current_tile = self.action_manager.analyze_tile(tiles[0])
            if current_tile["food"] > 0:
                if self.action_manager.take_object("food"):
                    self.logger.info("Found and took food from current tile")
                    return True

            # Check nearby tiles for food
            for i in range(1, len(tiles)):
                tile = self.action_manager.analyze_tile(tiles[i])
                if tile["food"] > 0:
                    # Move toward food (simplistic approach: just move forward)
                    self.action_manager.move_forward()
                    self.logger.info("Found food on nearby tile, moving toward it")
                    return True

        # No food visible, move randomly
        self.logger.info("No food visible, moving randomly")
        return self._perform_random_movement()

    def _handle_resource_collection(self):
        """Collect resources based on needs"""
        # Try to collect resources from current tile
        if self._collect_resources_from_tile():
            return True

        # Look around for resources
        tiles = self.action_manager.look_around()
        if not tiles:
            return self._perform_random_movement()

        # Check nearby tiles for needed resources
        found_resource = False
        for i in range(1, len(tiles)):
            tile = self.action_manager.analyze_tile(tiles[i])
            for resource_type, amount_needed in self.target_resources.items():
                resource_name = get_resource_name(resource_type)
                if self.inventory[resource_type] < amount_needed and tile[resource_name] > 0:
                    found_resource = True
                    self.logger.info(f"Found needed resource {resource_name} on nearby tile")
                    break
            if found_resource:
                break

        if found_resource:
            # Move toward resource (simplistic approach: just move forward)
            self.action_manager.move_forward()
            return True
        else:
            # No visible needed resources, move randomly
            return self._perform_random_movement()

    def _handle_elevation(self):
        """Handle the elevation process"""
        # First, check how many players are on our tile
        tiles = self.action_manager.look_around()
        if not tiles:
            self.state = "COLLECTING_RESOURCES"
            return False

        current_tile = self.action_manager.analyze_tile(tiles[0])
        required_players = ELEVATION_REQUIREMENTS[self.level][0]

        # If not enough players, broadcast for help
        if current_tile["players"] + 1 < required_players:
            self.comm_manager.broadcast_elevation_request(self.level)
            self.logger.info(f"Not enough players for elevation (have {current_tile['players'] + 1}, need {required_players}), broadcasting for help")

            # Wait for others or move on after timeout
            if time.time() - self.elevation_attempt_time > self.elevation_timeout:
                self.logger.info("Timeout waiting for elevation participants, returning to resource collection")
                self.state = "COLLECTING_RESOURCES"

            return False

        # Place resources for elevation
        if self._prepare_for_elevation():
            # Start incantation
            new_level = self.action_manager.incantation()
            if new_level > 0:
                self.level = new_level
                self.vision_range = new_level
                self._update_target_resources()

                # Broadcast our level up
                self.comm_manager.broadcast_level_up(self.level)

                self.logger.info(f"Elevation successful! New level: {self.level}")
                self.state = "COLLECTING_RESOURCES"
                return True
            else:
                self.logger.warning("Elevation failed!")
                self.state = "COLLECTING_RESOURCES"
                return False
        else:
            self.logger.warning("Failed to place all required resources for elevation")
            self.state = "COLLECTING_RESOURCES"
            return False

    def _prepare_for_elevation(self):
        """Place the required resources on the ground for elevation"""
        if self.level >= 8:
            return False

        _, linemate, deraumere, sibur, mendiane, phiras, thystame = ELEVATION_REQUIREMENTS[self.level]

        # Place the resources on the ground
        resources_placed = 0

        if linemate > 0 and self.inventory[ResourceType.LINEMATE] >= linemate:
            for _ in range(linemate):
                if self.action_manager.set_object("linemate"):
                    resources_placed += 1

        if deraumere > 0 and self.inventory[ResourceType.DERAUMERE] >= deraumere:
            for _ in range(deraumere):
                if self.action_manager.set_object("deraumere"):
                    resources_placed += 1

        if sibur > 0 and self.inventory[ResourceType.SIBUR] >= sibur:
            for _ in range(sibur):
                if self.action_manager.set_object("sibur"):
                    resources_placed += 1

        if mendiane > 0 and self.inventory[ResourceType.MENDIANE] >= mendiane:
            for _ in range(mendiane):
                if self.action_manager.set_object("mendiane"):
                    resources_placed += 1

        if phiras > 0 and self.inventory[ResourceType.PHIRAS] >= phiras:
            for _ in range(phiras):
                if self.action_manager.set_object("phiras"):
                    resources_placed += 1

        if thystame > 0 and self.inventory[ResourceType.THYSTAME] >= thystame:
            for _ in range(thystame):
                if self.action_manager.set_object("thystame"):
                    resources_placed += 1

        # Check if we placed all required resources
        expected_resources = linemate + deraumere + sibur + mendiane + phiras + thystame
        return resources_placed == expected_resources

    def _collect_resources_from_tile(self):
        """Collect resources from the current tile based on needs"""
        tile_contents = self.action_manager.check_resources_on_tile()
        resources_collected = 0

        # Always collect food first
        if tile_contents.get("food", 0) > 0:
            if self.action_manager.take_object("food"):
                resources_collected += 1
                self.logger.info("Collected food from current tile")
                return True

        # Collect other resources based on elevation requirements
        for resource_type, amount_needed in self.target_resources.items():
            resource_name = get_resource_name(resource_type)
            # If we need more of this resource and it's on the tile
            if (self.inventory[resource_type] < amount_needed and
                tile_contents.get(resource_name, 0) > 0):
                if self.action_manager.take_object(resource_name):
                    resources_collected += 1
                    self.logger.info(f"Collected {resource_name} from current tile")
                    return True

        return resources_collected > 0

    def _check_elevation_requirements(self):
        """Check if we can perform an elevation ritual"""
        if self.level >= 8:
            return False  # Max level reached

        # Check if we have the required resources in inventory
        required_players, linemate, deraumere, sibur, mendiane, phiras, thystame = ELEVATION_REQUIREMENTS[self.level]

        # Check number of players on the same tile (including self)
        tiles = self.action_manager.look_around()
        if not tiles:
            return False

        current_tile = tiles[0]  # The first tile is where we are standing
        tile_analysis = self.action_manager.analyze_tile(current_tile)

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

    def _perform_random_movement(self):
        """Perform a random movement to explore the map"""
        # Random choice: move forward or change direction
        choice = random.randint(0, 3)

        if choice == 0:  # Move forward
            return self.action_manager.move_forward()
        elif choice == 1:  # Turn right
            return self.action_manager.turn_right()
        elif choice == 2:  # Turn left
            return self.action_manager.turn_left()
        else:  # Look around
            self.action_manager.look_around()
            return True

    def broadcast_status(self):
        """Broadcast status to teammates occasionally"""
        if random.random() < 0.1:  # Limit broadcasting frequency
            # Format: "TEAM:ID:TYPE:DATA"
            if self.state == "ELEVATION":
                self.comm_manager.broadcast_elevation_request(self.level)
            elif self.state == "COLLECTING_RESOURCES":
                resources = ",".join([f"{get_resource_name(r)}:{self.inventory[r]}"
                                    for r in self.inventory
                                    if r != ResourceType.FOOD])
                self.comm_manager.broadcast_resources(resources)

    def get_state(self):
        """Get the current state"""
        return self.state

    def get_level(self):
        """Get the current level"""
        return self.level

    def update_level(self, level):
        """Update the current level"""
        if level != self.level:
            self.level = level
            self.vision_range = level
            self._update_target_resources()
            return True
        return False
