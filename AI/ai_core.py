#!/usr/bin/env python3
##
## EPITECH PROJECT, 2025
## Zappy
## File description:
## Core AI logic implementation
##

from concurrent.futures import process
import re
import math
import random
from itertools import cycle
import json
from collections import Counter
import re
from AI.logger import get_logger

MATERIALS = ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]

LEVEL_REQUIREMENTS = {1: {"linemate": 1},
        2: {"linemate": 1, "deraumere": 1, "sibur": 1},
        3: {"linemate": 2, "sibur": 1, "phiras": 2},
        4: {"linemate": 1, "deraumere": 1, "sibur": 2, "phiras": 1},
        5: {"linemate": 1, "deraumere": 2, "sibur": 1, "mendiane": 3},
        6: {"linemate": 1, "deraumere": 2, "sibur": 3, "phiras": 1},
        7: {"linemate": 2, "deraumere": 2, "sibur": 2, "mendiane": 2, "phiras": 2, "thystame": 1},
}

class AICore:
    """
    This class encapsulates the intelligence of the AI player.

    The AICore handles all game logic, decision making, resource management,
    inventory tracking, and coordination with other bots. It implements the
    strategies for gathering resources, performing rituals for level upgrades,
    and communicating with team members.
    """
    def __init__(self, name, bot_id=0, use_ui=False):
        """
        Initialize the AICore with default values.

        Args:
            name (str): The name of the team this AI belongs to
            bot_id (int, optional): The bot ID for logging. Defaults to 0.
            use_ui (bool, optional): Whether terminal UI is enabled. Defaults to False.

        Returns:
            None
        """
        self.backpack = {"food": 0, "linemate": 0, "deraumere": 0, "sibur": 0, "mendiane": 0, "phiras": 0, "thystame": 0}
        self.vision = ""
        self.message_received = []
        self.action_queue = []
        self.state = -2
        self.action = ""
        self.level = 1
        self.active = 1
        self.free_slots = 0
        self.team_backpack = {}
        self.bot_id = bot_id
        self.team_name = name
        self.found_new_item = False
        self.target_resource = ""
        self.ritual_mode = 0
        self.ritual_leader = 0
        self.players_for_ritual = 1
        self.source_direction = 9
        self.ritual_ready = 0
        self.clear_read_flag = 0
        self.clear_message_flag = 0
        self.reproduction = 0
        self.use_ui = use_ui
        self.logger = get_logger(bot_id=bot_id, team_name=name, log_to_console=not use_ui)
        self.fork = 1
        self.ready_bots = 0
        self.team_size = 1
        self.alive_bots = set()
        self.awaiting_alive = False

    def can_perform_ritual(self) -> bool:
        """
        Check if ritual is possible with the current team resources.

        This method determines whether the team has gathered enough resources
        to perform a ritual for the current level, considering both the team's
        existing inventory and the target resource.

        Returns:
            bool: True if ritual can be performed, False otherwise
        """
        needed_materials = LEVEL_REQUIREMENTS[self.level].copy()
        if "total" in self.team_backpack:
            items = self.team_backpack["total"].copy()
            if self.target_resource in items:
                items[self.target_resource] += 1
            else:
                items[self.target_resource] = 1
            for k in needed_materials:
                if needed_materials[k] > items[k]:
                    return False
            return True
        return False

    def find_needed_resource(self) -> str:
        """
        Determine which resource the AI should seek next.

        This method compares the team's current inventory with the requirements
        for the current level and identifies missing resources. If all required
        resources are available, it defaults to seeking food.

        Returns:
            str: The name of the resource to look for next
        """
        needed_materials = LEVEL_REQUIREMENTS[self.level].copy()
        resources_to_find = []
        if "total" in self.team_backpack:
            items = self.team_backpack["total"].copy()
        else:
            items = {"food": 0, "linemate": 0, "deraumere": 0, "sibur": 0, "mendiane": 0, "phiras": 0, "thystame": 0}

        self.logger.debug(f"Checking needed resources for level {self.level}")
        for k in needed_materials:
            item_count = items.get(k, 0)
            needed_count = needed_materials[k]
            if item_count < needed_count:
                resources_to_find.append(k)
                self.logger.debug(f"Need more {k}: have {item_count}, need {needed_count}")

        if resources_to_find == []:
            self.logger.info("All ritual resources collected, focusing on food")
            return "food"

        chosen_resource = random.choice(resources_to_find)
        self.logger.info(f"Looking for resource: {chosen_resource}")
        return chosen_resource


    def simple_team_hash(self, text: str) -> str:
        """
        Hash a message by concatenating the team name and the message, then returning a simple checksum (sum of ASCII codes modulo 256).

        Args:
            text (str): The plain text to hash

        Returns:
            str: The hashed message as a string
        """
        # Pour le broadcast, on ne doit pas avoir plus d'un ':'
        # On encode tout dans un seul argument, sans espace, point-virgule ou JSON
        # Format: checksum_type_data (ex: 123_Alive_3)
        if text.startswith("Alive:"):
            # Alive:{bot_id} => Alive_{bot_id}
            text = text.replace(":", "_")
        elif text.startswith("inventory"):
            # inventory{bot_id};{level};{json.dumps(self.backpack)}
            # => inventory_{bot_id}_{level}_{backpackcompact}
            parts = text.split(";")
            if len(parts) == 3:
                inv = parts[2].replace(" ", "").replace(",", "-").replace(":", "-").replace("{", "").replace("}", "")
                text = f"inventory_{parts[0][9:]}_{parts[1]}_{inv}"
        elif text.startswith("pose_ressources"):
            text = text.replace("_", "").replace(":", "").replace(";", "")
        elif text.startswith("ready"):
            text = text.replace(":", "_")
        elif text.startswith("on my way"):
            text = text.replace(" ", "_")
        elif text.startswith("incantation") or ";incantation;" in text:
            # ex: '3;incantation;2' => 'incantation_3_2'
            parts = text.split(";")
            if len(parts) == 3:
                text = f"incantation_{parts[0]}_{parts[2]}"
        data = (self.team_name + text)
        checksum = sum(ord(c) for c in data) % 256
        return f"{checksum}:{text}"

    def simple_team_unhash(self, hashed: str) -> str:
        """
        Retrieve the message if the checksum matches, else return an empty string.
        Handles trailing newlines or spaces.
        Args:
            hashed (str): The hashed message

        Returns:
            str: The original message if valid, else empty string
        """
        try:
            hashed = hashed.strip()
            checksum, text = hashed.split(":", 1)
            data = (self.team_name + text)
            if int(checksum) == sum(ord(c) for c in data) % 256:
                return text
            else:
                return ""
        except Exception:
            return ""

    def parse_backpack(self, data):
        """
        Parse the inventory data received from the server.

        This method processes the formatted inventory data string from the server
        and updates the backpack dictionary with the current inventory quantities.

        Args:
            data (str): The inventory data string from the server

        Returns:
            None
        """
        old_backpack = self.backpack.copy()
        for char in "[]":
            data = data.replace(char, "")
        data = data.split(",")
        for i in range(len(data)):
            data[i] = data[i][1:]
        data[len(data) - 1] = data[len(data) - 1][:-1]
        for elem in data:
            if elem:
                resource = elem.split()[0]
                quantity = int(elem.split()[1])
                self.backpack[resource] = quantity

        self.logger.log_inventory(self.backpack)
        for key in self.backpack:
            if key in old_backpack and self.backpack[key] > old_backpack[key]:
                self.logger.info(f"Gained {self.backpack[key] - old_backpack[key]} {key}")
            elif key in old_backpack and self.backpack[key] < old_backpack[key]:
                self.logger.info(f"Used/lost {old_backpack[key] - self.backpack[key]} {key}")

    def update_team_backpack(self, data):
        """
        Update team inventory data with broadcast information from other players.

        This method processes inventory broadcasts from team members and maintains
        a collective inventory of all team resources.

        Args:
            data (str): Formatted broadcast data from another player

        Returns:
            None
        """
        bot_id, _, backpack = data.split(";")
        self.team_backpack[self.bot_id] = self.backpack
        self.team_backpack[bot_id] = json.loads(backpack)
        c = Counter()
        for d in self.team_backpack:
            if d == 'total':
                continue
            c.update(self.team_backpack[d])
        self.team_backpack['total'] = dict(c)

    def refresh_team_inventory(self):
        """
        Update the team's collective inventory after acquiring a new object.

        This method updates the team inventory with the AI's current backpack
        contents and recalculates the total resources available to the team.

        Returns:
            None
        """
        self.team_backpack[self.bot_id] = self.backpack
        c = Counter()
        for d in self.team_backpack:
            if d == 'total':
                continue
            c.update(self.team_backpack[d])
        self.team_backpack['total'] = dict(c)


    def get_vision_size(self, grid: list) -> int:
        """
        Calculate the size of the vision grid.

        This method determines the number of non-empty rows in the vision grid.

        Args:
            grid (list): The vision grid to analyze

        Returns:
            int: The number of non-empty rows in the grid
        """
        count = 0
        for elem in grid:
            if elem == []:
                return count
            count += 1
        return count

    def locate_resource(self, grid: list, resource: str) -> list:
        """
        Search the vision grid for a specific resource.

        This method systematically scans the vision grid to locate the coordinates
        of a specified resource, searching in concentric squares outward from the center.

        Args:
            grid (list): The vision grid to search in
            resource (str): The name of the resource to find

        Returns:
            list: Coordinates [v, h] of the resource if found, empty list otherwise
        """

        v = 8
        h = 0
        while h < self.get_vision_size(grid[v]):
            if grid[v][h] != [] and resource in grid[v][h][0]:
                return [v, h]
            else:
                h += 1
            tv = v
            while tv >= v-h:
                if grid[v][h] != [] and resource in grid[tv][h][0]:
                    return [tv, h]
                tv -= 1
            tv = v
            while tv <= v+h:
                if grid[v][h] != [] and resource in grid[tv][h][0]:
                    return [tv, h]
                tv += 1
            h+=1
        return []

    def count_vision_lines(self, data: list) -> int:
        """
        Calculate the number of lines in the vision data.

        This method determines the square root of the data length to find
        the number of lines in the vision field.

        Args:
            data (list): The raw vision data array

        Returns:
            int: The number of lines in the vision field
        """
        data_len = len(data)
        return int(math.sqrt(data_len))

    def construct_vision_grid(self, grid: list, data: list) -> list:
        """
        Construct a 2D vision grid from the raw vision data.

        This method transforms the linear vision data into a 2D grid format
        representing the AI's visual field, with positions of objects mapped
        according to their relative locations.

        Args:
            grid (list): The empty grid structure to fill
            data (list): The raw vision data containing object positions

        Returns:
            list: The populated 2D vision grid
        """
        count = 1
        v = 8
        h = 0
        i = 0

        line = self.count_vision_lines(data)
        for j in range(line):
            tv = v - h
            for a in range(count):
                grid[tv][h].append(data[i])
                tv += 1
                i += 1
            count = (count + 2)
            h += 1
        return grid

    def create_empty_grid(self) -> list:
        """
        Generate an empty map
        Returns:
            array: the empty map
        """
        return [[[] for i in range(9)] for j in range(17)]

    def split_vision_data(self, data: str) -> list:
        """
        Split the look command
        Args:
            data (str): the look command
        Returns:
            list: the object split"""
        result = data
        for char in "[]":
            result = result.replace(char, "")
        result_list = result.split(",")
        for i in range(len(result_list)):
            result_list[i] = result_list[i][1:]
        result_list[len(result_list) - 1] = result_list[len(result_list) - 1][:-2]
        return result_list

    def analyze_vision(self, data: str, resource : str) -> list:
        """
        Parse the look command
        Args:
            data (str): the look command
            resource (str): the resource to look for
        Returns:
            array: the list of object position
        """
        data1 = data.split(",")
        elements = []
        for i in range(len(data1)):
            elements.append(' '.join(re.split('[^a-zA-Z0-9]', data1[i])[1:]))
        grid = self.create_empty_grid()
        grid = self.construct_vision_grid(grid, elements)
        location = self.locate_resource(grid, resource)
        actions = []

        if not location or len(location) < 2:
            self.logger.debug(f"{resource} not found in vision, moving randomly")
            actions.append(random.choice(["Forward\n", "Right\n", "Left\n"]))
            actions.append(random.choice(["Forward\n", "Right\n", "Left\n"]))
            actions.append(random.choice(["Forward\n", "Right\n", "Left\n"]))
            return actions
        elif location[0] == 8 and location[1] == 0:
            self.logger.log_resource_found(resource)
            self.logger.debug(f"Found {resource} at current position")
            return ["Take " + resource + "\n"]
        else:
            self.logger.log_resource_found(resource)
            self.logger.debug(f"Found {resource} at relative position [{location[0]}, {location[1]}]")

            for i in range(int(location[0]) - 8):
                actions.append("Forward\n")
            if location[0] == 8:
                for i in range(int(location[1])):
                    actions.append("Forward\n")
                actions.append("Take " + resource + "\n")
                actions.append("Inventory\n")
            if location[1] == 0:
                actions.append("Take " + resource + "\n")
            if int(location[0]) < 8:
                actions.append("Left\n")
                for i in range(8 - int(location[0])):
                    actions.append("Forward\n")
                actions.append("Take " + resource + "\n")
                actions.append("Inventory\n")
            if int(location[0]) > 8:
                actions.append("Right\n")
                for i in range(int(location[0]) - 8):
                    actions.append("Forward\n")
                actions.append("Take " + resource + "\n")
                actions.append("Inventory\n")
        return actions

    def handle_message(self, message):
        direction = int(message[8])
        encrypted_message = message[11:]
        decrypted_message = self.simple_team_unhash(encrypted_message)
        self.logger.log_message(decrypted_message, direction)

        # Gestion du recensement Alive
        if decrypted_message.startswith("Alive_"):
            sender_id = int(decrypted_message.split("_")[1])
            if self.ritual_leader >= 1 and self.awaiting_alive:
                self.alive_bots.add(sender_id)
                self.logger.info(f"Alive reçu de {sender_id}, total: {len(self.alive_bots)}")
            elif self.ritual_leader < 1:
                # Tous les bots répondent, même hors élévation
                reply = self.simple_team_hash(f"Alive_{self.bot_id}")
                self.action = f"Broadcast {reply}\n"
                self.logger.info(f"Bot {self.bot_id} répond 'Alive' au leader")
            return

        if decrypted_message.startswith("inventory_"):
            self.logger.debug("Received inventory update from team member")
            # Optionnel: parser si besoin
            return

        if decrypted_message.startswith("incantation_"):
            sender_id = int(decrypted_message.split("_")[1])
            sender_level = int(decrypted_message.split("_")[2])

            if sender_level != self.level:
                return

            if self.ritual_leader >= 1 and sender_id > self.bot_id:
                self.logger.info(f"Canceling ritual leadership: Bot {sender_id} has higher priority")
                self.ritual_leader = 0
                self.ritual_mode = 0
                self.state = 0
                return

            if self.state > -1 and self.state < 4 and self.backpack["food"] > 35:
                self.logger.info("Switching to ritual mode")
                self.state = 4
                self.action_queue = []

            if self.ritual_mode == 1:
                self.logger.info(f"Moving toward ritual source in direction {direction}")
                self.action_queue = self.move_to_message_source(direction)

        if "pose_ressources" in decrypted_message:
            self.logger.info("Ordre reçu : poser les ressources pour le rituel")
            self.ritual_mode = 2
            self.state = 6

        if "on my way" in decrypted_message and self.ritual_leader >= 1:
            self.players_for_ritual += 1
            self.logger.info(f"Bot joining ritual, total: {self.players_for_ritual}")

            if self.state in [6, 7] and self.check_resources_on_tile():
                players_needed = self.get_required_players_for_level()
                available_players = self.get_available_players_count()

                if available_players >= players_needed:
                    self.logger.info(f"New bot joined, now have enough players ({available_players}/{players_needed})")
                    if self.state == 7:
                        self.begin_ritual()

        if "ready" in decrypted_message and self.ritual_leader >= 1:
            self.ready_bots += 1
            total_ready = self.ready_bots
            if self.ritual_leader == self.bot_id or self.ritual_leader >= 1:
                total_ready += 1
            self.logger.info(f"Bot ready for ritual, ready bots (including leader): {total_ready}")

            players_needed = self.get_required_players_for_level()
            self.logger.info(f"Players needed for ritual: {players_needed}")
            self.logger.info(f"Total ready bots (including leader): {total_ready}")
            self.logger.info(f"State: {self.state}")
            if total_ready >= (players_needed - 1) and self.state == 6:
                if self.check_resources_on_tile():
                    self.begin_ritual()

    def move_to_message_source(self, direction: int) -> list:
        if self.ritual_ready == 1 or self.action_queue:
            return []
        actions = []
        if (direction == 0):
            message = self.simple_team_hash("ready")
            self.action = "Broadcast " + message + "\n"
            self.source_direction = 0
            self.ritual_ready = 1
            return []
        if (direction in (2, 1, 8)):
            actions.append("Forward\n")
        elif (direction in (5, 6, 7)):
            actions.append("Right\n")
        else:
            actions.append("Left\n")
        return actions

    def place_resources_for_ritual(self):
        self.logger.info("Placing resources for ritual...")
        self.logger.info(f"Ritual leader: {self.ritual_leader}, Ritual mode: {self.ritual_mode}")
        if self.action_queue:
            return
        if self.ritual_leader < 1 and self.ritual_mode != 2:
            self.logger.info("En attente de l'ordre du leader pour poser les ressources...")
            return
        if self.ritual_leader >= 1 and self.ritual_mode != 2:
            message = self.simple_team_hash("pose_ressources")
            self.action = "Broadcast " + message + "\n"
            self.ritual_mode = 2
            self.logger.info("Leader : envoi de l'ordre de poser les ressources (état 6)")
            self.state = 6
            return
        data = self.vision.split(",")[0]
        while True:
            if len(data) == 0 or data[0].isalpha():
                break
            data = data[1:]
        data = data.split(" ")
        needed_materials = LEVEL_REQUIREMENTS[self.level].copy()

        self.logger.debug(f"Resources on tile: {data}")

        for k in needed_materials:
            for j in data:
                if j == k:
                    needed_materials[k] -= 1

        missing_resources = {k: v for k, v in needed_materials.items() if v > 0}
        if missing_resources:
            self.logger.debug(f"Missing resources for ritual: {missing_resources}")

        for k in needed_materials:
            if needed_materials[k] < 1:
                continue
            if k in self.backpack and self.backpack[k] != 0:
                self.action_queue = ["Set " + k + "\n"]
                self.action_queue.append("Look\n")
                self.backpack[k] -= 1
                self.logger.info(f"Placing resource for ritual: {k}")
                return

        self.state = 7
        self.action = ""
        self.logger.info("All resources placed for ritual, ready for incantation")

        players_present = self.count_players_on_tile()
        players_needed = self.get_required_players_for_level()

        self.players_for_ritual = max(self.players_for_ritual, players_present)

        self.logger.info(f"Players present for ritual: {players_present}/{players_needed}")

        if players_present >= players_needed:
            self.logger.info("Enough players physically present, starting incantation!")
            self.begin_ritual()
        elif players_present >= 1 and self.ritual_leader >= 1:
            adjusted_needed = self.get_required_players_for_level()
            if players_present >= adjusted_needed:
                self.logger.info(f"Adapting ritual to available players: {players_present}/{players_needed}")
                self.begin_ritual()
        return

    def begin_ritual(self):

        players_present = self.count_players_on_tile()
        players_needed = self.get_required_players_for_level()

        if players_present < players_needed:
            self.logger.warning(f"Cannot begin ritual: not enough players present ({players_present}/{players_needed})")
            return


        data = self.vision.split(",")[0]
        while True:
            if len(data) == 0 or data[0].isalpha():
                break
            data = data[1:]
        data = data.split(" ")


        self.logger.debug(f"Tile contents before ritual: {data}")

        needed_materials = LEVEL_REQUIREMENTS[self.level].copy()
        for k in needed_materials:
            for j in data:
                if j == k:
                    needed_materials[k] -= 1


        missing = {k: v for k, v in needed_materials.items() if v > 0}
        if missing:
            self.logger.warning(f"Cannot begin ritual: missing resources {missing}")
            return

        self.logger.info(f"Beginning incantation ritual for level {self.level} with {players_present} players")


        self.action_queue = ["Incantation\n"]
        self.action = "Incantation\n"
        self.state = 8

    def decide_action(self):
        """
        Determine the next action for the AI to take.

        This method is the main decision-making function that chooses the
        most appropriate action based on the current game state, resources,
        inventory, and team needs.

        Returns:
            None: Updates the action attribute instead of returning a value
        """
        if self.state == -2:
            self.logger.info("Initializing: checking team connections")
            self.action = "Connect_nbr\n"
            self.state += 1
        elif self.state == -1:
            self.logger.info("Initializing: looking at surroundings")
            self.action = "Look\n"
            self.state += 1
        elif self.state == 0:
            if self.action_queue:
                self.action = self.action_queue[0]
                self.action_queue = self.action_queue[1:]
                self.logger.debug(f"Executing queued action: {self.action.strip()}")
            else:
                self.logger.debug("Checking inventory")
                self.action = "Inventory\n"
                self.state += 1
        elif self.state == 1:
            if self.found_new_item:
                if not self.can_perform_ritual():
                    message = self.simple_team_hash("inventory" + str(self.bot_id) + ";" + str(self.level) + ";" + str(json.dumps(self.backpack)))
                    self.action = "Broadcast " + message + "\n"
                else:
                    message = self.simple_team_hash(str(self.bot_id) + ";incantation;" + str(self.level))
                    self.action = "Broadcast " + message + "\n"
                    self.ritual_leader = 1
                    self.state = 4
                    self.ritual_mode = 1
                    self.found_new_item = False
                    return
                self.found_new_item = False
            else:
                self.state += 1
                self.decide_action()
                return
            self.state += 1
        elif self.state == 2:
            self.action = "Look\n"
            self.state += 1
        elif self.state == 3:
            if "food" in self.backpack and self.backpack["food"] < 45:
                self.action_queue = self.analyze_vision(self.vision, "food")
                self.state = 0
            else:
                self.target_resource = self.find_needed_resource()
                self.action_queue = self.analyze_vision(self.vision, self.target_resource)
                self.state = 0
        elif self.state == 4:
            if (
                self.ritual_leader >= 1
                and self.ritual_mode == 1
                and self.ready_bots >= self.get_required_players_for_level() - 1
                and self.count_players_on_tile() >= len(self.alive_bots)
            ):
                self.logger.info(f"Number alive bots: {self.alive_bots}, ready bots: {self.ready_bots}")
                self.logger.info("Tous les bots vivants sont prêts ET présents, passage en state 6 pour poser les ressources")
                self.logger.info(f"Ritual leader: {self.ritual_leader}, Ritual mode: {self.ritual_mode}",
                                 f"Ready bots: {self.ready_bots}, Alive bots: {len(self.alive_bots)}",
                                 f"Players on tile: {self.count_players_on_tile()}")
                self.state = 6
                return
            if self.ritual_mode == 0:
                data = self.simple_team_hash(str(self.bot_id) + " on my way")
                self.action = "Broadcast " + data + "\n"
                self.ritual_mode = 1
                return
            if self.ritual_leader >= 6:
                self.state += 1
                return
            if self.ritual_leader >= 1:
                pass
            elif self.action_queue and not self.ritual_ready:
                self.action = self.action_queue[0]
                self.action_queue = self.action_queue[1:]
                self.clear_message_flag = 1
                if not self.action_queue:
                    self.clear_read_flag = 1
            elif self.ritual_ready and "Broadcast" in self.action:
                self.state += 1
            else:
                self.action = ""
        elif self.state == 5:
            self.clear_message_flag = 0
            self.action = "Look\n"
            self.state += 1
        elif self.state == 6:
            if self.ritual_leader >= 1 and self.ritual_mode != 2:
                message = self.simple_team_hash("pose_ressources")
                self.action = "Broadcast " + message + "\n"
                self.ritual_mode = 2
                self.logger.info("Leader : envoi de l'ordre de poser les ressources (état 6)")
                self.state = 6  # Ajout : le leader passe en step 6 après l'ordre
                return

            if self.vision:
                players_needed = self.get_required_players_for_level()
                players_present = self.count_players_on_tile()
                resources_ready = self.check_resources_on_tile()

                self.players_for_ritual = max(self.players_for_ritual, players_present)

                self.logger.info(f"Ritual status: {players_present} players présents sur {players_needed} nécessaires")
                self.logger.info(f"Ressources pour incantation prêtes: {resources_ready}")

                if players_present >= 5 and resources_ready:
                    self.logger.info("Force: 5 joueurs présents, lancement de l'incantation !")
                    self.begin_ritual()
                    return

                if players_present >= players_needed and resources_ready:
                    self.logger.info(f"Toutes les conditions remplies pour l'incantation: {players_present}/{players_needed} joueurs, ressources OK")
                    self.begin_ritual()
                    return

            players_needed = self.get_required_players_for_level()
            available_players = self.get_available_players_count()

            self.logger.info(f"Ritual status: {available_players} players available out of {players_needed} needed")

            if self.ritual_leader >= 1 and available_players >= players_needed:
                self.begin_ritual()

            if self.state != 8:
                self.place_resources_for_ritual()
                if self.action_queue:
                    self.action = self.action_queue[0]
                    self.action_queue = self.action_queue[1:]
                else:
                    self.action = "Look\n"
        elif self.state == 7:

            if self.vision:
                players_present = self.count_players_on_tile()
                resources_ready = self.check_resources_on_tile()
                players_needed = self.get_required_players_for_level()

                self.players_for_ritual = max(self.players_for_ritual, players_present)

                self.logger.info(f"État 7: {players_present} joueurs présents sur {players_needed} nécessaires, ressources prêtes: {resources_ready}")


                if players_present >= players_needed and resources_ready:
                    self.logger.info("Conditions remplies pour lancer l'incantation!")
                    self.begin_ritual()
                    return


            players_needed = self.get_required_players_for_level()
            available_players = self.get_available_players_count()


            if self.ritual_leader >= 1 and available_players < players_needed:

                adjusted_needed = self.get_required_players_for_level()
                if adjusted_needed <= available_players:
                    self.logger.info(f"Adapting ritual to available players: {available_players}/{players_needed}")
                    self.begin_ritual()
                    return

            if self.action_queue:
                self.action = self.action_queue[0]
                self.action_queue = self.action_queue[1:]
            else:

                self.action = "Look\n"
        elif self.state == 8:

            self.action = ""
        elif self.state == 9:

            self.action = "Inventory\n"
            self.action_queue = []
            self.target_resource = ""
            self.ritual_mode = 0
            self.ritual_leader = 0
            self.ritual_ready = 0
            self.players_for_ritual = 1
            self.state += 1
        elif self.state == 10:

            message = self.simple_team_hash(f"inventory_{self.bot_id}_{self.level}_{str(self.backpack).replace(' ', '').replace(',', '-')}")
            self.action = "Broadcast " + message + "\n"
            self.state = 0

    def count_team_members(self):
        """
        Lance un recensement de la team via broadcast 'Alive'.
        Le leader envoie 'Alive', les autres répondent, et on compte les réponses.
        """
        self.alive_bots = set([self.bot_id])  # Le leader s'ajoute lui-même
        self.awaiting_alive = True
        message = self.simple_team_hash(f"Alive_{self.bot_id}")  # Format corrigé
        self.action = f"Broadcast {message}\n"
        self.logger.info("Leader: broadcast 'Alive' pour recensement de la team")

    def get_required_players_for_level(self) -> int:
        """
        Determine the number of players needed for the current level ritual,
        adapting to the actual number of available players.

        Returns:
            int: Number of players needed for the current level
        """

        theoretical_required = self.level + 1


        available_players = self.get_available_players_count()


        if available_players < theoretical_required:

            if self.level == 1 and available_players == 1:
                self.logger.info("Solo elevation attempt for level 1")
                return 1


            if available_players >= 2 and self.level < 4:
                self.logger.info(f"Adapting ritual requirements: using {available_players} players instead of {theoretical_required}")
                return available_players


        return theoretical_required

    def count_players_on_tile(self) -> int:
        """
        Compte le nombre de joueurs sur la même case que l'IA.

        Returns:
            int: nombre de joueurs sur la case
        """
        if not self.vision:
            return 0

        try:
            current_tile = self.vision.split(",")[0]

            player_count = current_tile.count("player")

            total_players = player_count + 1

            self.logger.debug(f"Detected {player_count} other players on current tile, total with self: {total_players}")

            return total_players
        except Exception as e:
            self.logger.error(f"Erreur lors du comptage des joueurs: {e}")
            return 1

    def check_resources_on_tile(self) -> bool:
        """
        Vérifie si toutes les ressources nécessaires sont présentes sur la case courante.

        Returns:
            bool: True si toutes les ressources sont présentes, False sinon
        """
        if not self.vision:
            return False

        try:
            current_tile = self.vision.split(",")[0]
            needed_materials = LEVEL_REQUIREMENTS[self.level].copy()


            resource_counts = {}
            for k in needed_materials:
                count = current_tile.count(k)
                resource_counts[k] = count
                if count < needed_materials[k]:
                    self.logger.warning(f"Incantation impossible: {needed_materials[k]} {k} nécessaire, seulement {count} présent")
                    return False

            self.logger.debug(f"Resources check: {resource_counts}, required: {needed_materials}")
            return True
        except Exception as e:
            self.logger.error(f"Erreur lors de la vérification des ressources: {e}")
            return False

    def get_available_players_count(self) -> int:
        """
        Détermine le nombre d'IAs disponibles pour l'élévation en analysant
        les messages reçus et le nombre de joueurs visibles.

        Returns:
            int: Nombre estimé de joueurs disponibles pour l'élévation
        """

        players_on_tile = self.count_players_on_tile()


        if players_on_tile > 1:
            return players_on_tile


        comm_players = max(self.players_for_ritual, self.ritual_leader)


        team_slots_used = 6 - self.free_slots
        if team_slots_used > 0:

            result = min(team_slots_used, comm_players)
            self.logger.debug(f"Estimating available players: {result} (from slots: {team_slots_used}, comms: {comm_players})")
            return result


        return max(1, comm_players)
