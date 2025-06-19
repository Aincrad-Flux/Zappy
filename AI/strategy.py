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
        self.client_num = comm_manager.client_num

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

        # Beacon system
        self.is_beacon = False
        self.beacon_broadcast_interval = 1  # seconds
        self.last_beacon_broadcast = 0
        self.current_beacon_id = None
        self.beacon_follow_timeout = 30  # seconds
        self.beacon_follow_start_time = 0
        self.found_beacon_tile = False

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

        # Update the UI with inventory information
        self.comm_manager.update_inventory(self.inventory)

        # If food is critically low, prioritize finding food
        if self.inventory[ResourceType.FOOD] <= 5:
            self.state = "SEARCHING_FOOD"
            self.logger.info("Food critically low, switching to SEARCHING_FOOD state")
            return

        # Check if we're already following a beacon
        if self.state == "FOLLOWING_BEACON" and self.current_beacon_id:
            # Check if we've timed out
            if time.time() - self.beacon_follow_start_time > self.beacon_follow_timeout:
                self.logger.info(f"Timed out following beacon {self.current_beacon_id}, returning to resource collection")
                self.current_beacon_id = None
                self.state = "COLLECTING_RESOURCES"
            else:
                return  # Continue following beacon

        # Check if we are a beacon and update UI
        if self.is_beacon:
            self.state = "ACTING_AS_BEACON"
            self.comm_manager.set_beacon_mode(True, self.level)
            return
        else:
            self.comm_manager.set_beacon_mode(False)

        # Check if there's an active beacon for our level
        active_beacon = self.comm_manager.get_active_beacon(self.level)
        if active_beacon and active_beacon != self.client_num:
            self.logger.info(f"Found active beacon from player #{active_beacon}, following")
            self.state = "FOLLOWING_BEACON"
            self.current_beacon_id = active_beacon
            self.beacon_follow_start_time = time.time()
            return

        # Check if we can perform an elevation ritual
        if self._check_elevation_requirements():
            # For levels 3+, become a beacon instead of immediately elevating
            if self.level >= 2:  # Starting at level 2 going to 3+
                self.logger.info("All elevation requirements met for level 3+, becoming a beacon")
                self.is_beacon = True
                self.state = "ACTING_AS_BEACON"
            else:
                # For levels 1-2, perform regular elevation
                self.state = "ELEVATION"
                self.logger.info("All elevation requirements met for level 1-2, switching to ELEVATION state")
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
        elif self.state == "ACTING_AS_BEACON":
            self.logger.debug("Executing ACTING_AS_BEACON strategy")
            return self._handle_beacon_mode()
        elif self.state == "FOLLOWING_BEACON":
            self.logger.debug("Executing FOLLOWING_BEACON strategy")
            return self._handle_follow_beacon()

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

    def _handle_beacon_mode(self):
        """Act as a beacon for team evolution"""
        # Broadcast our beacon status periodically
        current_time = time.time()
        if current_time - self.last_beacon_broadcast > self.beacon_broadcast_interval:
            self.comm_manager.broadcast_beacon(self.level)
            self.last_beacon_broadcast = current_time
            self.logger.info(f"Broadcasting as beacon for level {self.level}")

            # Update the terminal UI
            self.comm_manager.set_beacon_mode(True, self.level)

        # Don't move while acting as a beacon - just stand still

        # Check if we have enough participants for elevation
        team_info = self.comm_manager.get_team_member_info()
        participants = [p for p in team_info['participants'].values()
                        if p['type'] == 'BEACON_RESPONSE' and
                           p['level'] == self.level and
                           time.time() - p['timestamp'] < 5]  # Only count recent responses

        required_players = ELEVATION_REQUIREMENTS[self.level][0]

        # Count participants (include self)
        total_participants = len(participants) + 1

        # If we have enough participants and they're close (on our tile)
        tiles = self.action_manager.look_around()
        if tiles:
            current_tile = self.action_manager.analyze_tile(tiles[0])
            players_on_tile = current_tile["players"] + 1  # +1 for ourselves

            if players_on_tile >= required_players:
                self.logger.info(f"Sufficient players on tile ({players_on_tile}/{required_players}), proceeding with elevation")
                self.is_beacon = False
                self.state = "ELEVATION"
                return self._handle_elevation()
            elif total_participants >= required_players:
                # We have enough responses but not everyone is here yet
                self.logger.info(f"Have {total_participants}/{required_players} participants, but only {players_on_tile} on tile. Continuing to broadcast.")

        return True

    def _handle_follow_beacon(self):
        """Follow a beacon signal to join for evolution"""
        # Get the direction to the beacon
        beacon_direction = self.comm_manager.get_beacon_direction(self.current_beacon_id)

        if beacon_direction is None:
            self.logger.warning("Lost signal from beacon, returning to resource collection")
            self.current_beacon_id = None
            self.state = "COLLECTING_RESOURCES"
            return False

        # Log information about the beacon direction
        direction_labels = ["Behind", "Behind-Right", "Right", "Front-Right", "Front", "Front-Left", "Left", "Behind-Left"]
        self.logger.info(f"Following beacon #{self.current_beacon_id} - Direction: {direction_labels[beacon_direction]} ({beacon_direction})")

        # Send our response to the beacon
        self.comm_manager.broadcast_beacon_response(self.current_beacon_id, self.level)

        # Look around to check if we're on the same tile as the beacon
        tiles = self.action_manager.look_around()
        if tiles:
            current_tile = self.action_manager.analyze_tile(tiles[0])

            # Check if there's a player on our tile who might be the beacon
            if current_tile["players"] > 0:
                # We might be on the beacon's tile, send a response and wait
                self.logger.info("Potentially found beacon's tile, waiting for elevation")
                self.found_beacon_tile = True
                return True

        # If we think we've found the beacon's tile, don't move anymore
        if self.found_beacon_tile:
            return True

        # Otherwise, move in the direction of the beacon
        # The beacon_direction is a number 0-7 representing the sound direction
        # We need to convert this to a movement command

        # Sound direction mapping (approximately):
        # 0: behind
        # 1: behind-right
        # 2: right
        # 3: front-right
        # 4: front
        # 5: front-left
        # 6: left
        # 7: behind-left

        if beacon_direction == 4:  # Front
            return self.action_manager.move_forward()
        elif beacon_direction in [3, 5]:  # Front-right or front-left
            return self.action_manager.move_forward()
        elif beacon_direction == 2:  # Right
            return self.action_manager.turn_right() and self.action_manager.move_forward()
        elif beacon_direction == 6:  # Left
            return self.action_manager.turn_left() and self.action_manager.move_forward()
        elif beacon_direction in [0, 1, 7]:  # Behind
            # Turn around (two right turns) and move
            return self.action_manager.turn_right() and self.action_manager.turn_right() and self.action_manager.move_forward()

        # If we can't determine direction, move randomly
        return self._perform_random_movement()

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
        # Always update UI with inventory and state
        self.comm_manager.update_inventory(self.inventory)

        # Is beacon status is determined in decide_next_action and _handle_beacon_mode

        if random.random() < 0.1:  # Limit broadcasting frequency
            # Format: "TEAM:ID:TYPE:DATA"
            if self.state == "ELEVATION":
                self.comm_manager.broadcast_elevation_request(self.level)
            elif self.state == "ACTING_AS_BEACON":
                # Beacons broadcast more frequently via _handle_beacon_mode
                pass
            elif self.state == "FOLLOWING_BEACON":
                # Already sending responses in _handle_follow_beacon
                pass
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
