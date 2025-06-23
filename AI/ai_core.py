from concurrent.futures import process
import re
import math
import random
from itertools import cycle
import json
from collections import Counter
import re

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
    """The AICore class encapsulates the intelligence of the player"""
    def __init__(self, name):
        """The AICore class encapsulates the intelligence of the player
        Args:
            client (NetworkClient): the client which helps you to communicate with the server.
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
        self.bot_id = 0
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
        self.reproduction = 1

    def can_perform_ritual(self) -> bool:
        """Check if ritual is possible with the new object and the team backpack

        Args:
            object (str): The name of the new objet we got

        Returns:
            bool: Ritual is possible or not
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

    def find_needed_resource(self):
        needed_materials = LEVEL_REQUIREMENTS[self.level].copy()
        resources_to_find = []
        if "total" in self.team_backpack:
            items = self.team_backpack["total"].copy()
        else:
            items = {"food": 0, "linemate": 0, "deraumere": 0, "sibur": 0, "mendiane": 0, "phiras": 0, "thystame": 0}
        for k in needed_materials:
            if k not in items or needed_materials[k] > items[k]:
                resources_to_find.append(k)
        if resources_to_find == []:
            return "food"
        return random.choice(resources_to_find)


    def xor_encrypt(self, key: str, text: str):
        """XOR encrypt two strings"""
        return ''.join(chr(ord(c)^ord(k)) for c,k in zip(text, cycle(key)))

    def parse_backpack(self, data):
        for char in "[]":
            data = data.replace(char, "")
        data = data.split(",")
        for i in range(len(data)):
            data[i] = data[i][1:]
        data[len(data) - 1] = data[len(data) - 1][:-1]
        for elem in data:
            if elem:
                self.backpack[elem.split()[0]] = int(elem.split()[1])

    def update_team_backpack(self, data):
        """Update team backpack with broadcast
                        elif "Take" in self.agent.action and "food" not in self.agent.action:
                            print("Failed to take ", self.agent.target_resource)
        Args:
            data (str): broadcast from other player
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
        """Update team backpack when we took new object
        """
        self.team_backpack[self.bot_id] = self.backpack
        c = Counter()
        for d in self.team_backpack:
            if d == 'total':
                continue
            c.update(self.team_backpack[d])
        self.team_backpack['total'] = dict(c)


    def get_vision_size(self, grid: list) -> int:
        """Get the size of the array
        Args:
            grid (list): the array
        Returns:
            int: the size of the array
        """
        count = 0
        for elem in grid:
            if elem == []:
                return count
            count += 1
        return count

    def locate_resource(self, grid: list, resource: str) -> list:
        """Find the resource in the map
        Args:
            grid (list): the map
            resource (str): the resource
        Returns:
            list: coord x y of the resource
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
        return []  # Return empty list instead of None

    def count_vision_lines(self, data: list) -> int:
        """Get the number of line in the array
        Args:
            data (list): the array
        Returns:
            int: the number of line
        """
        data_len = len(data)
        return int(math.sqrt(data_len))

    def construct_vision_grid(self, grid: list, data: list) -> list:
        """Fill the map with the object position
        Args:
            grid (list): the map
            data (list): the array of object position
        return :
            array: the map
        """
        count = 1
        v = 8
        h = 0
        i = 0
        x = 0

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
        """Generate an empty map
        Returns:
            array: the empty map
        """
        return [[[] for i in range(9)] for j in range(17)]

    def split_vision_data(self, data: str) -> list:
        """Split the look command
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
        """Parse the look command
        Args:
            data (str): the look command
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
            actions.append(random.choice(["Forward\n", "Right\n", "Left\n"]))
            actions.append(random.choice(["Forward\n", "Right\n", "Left\n"]))
            actions.append(random.choice(["Forward\n", "Right\n", "Left\n"]))
            return actions
        elif location[0] == 8 and location[1] == 0:
            return ["Take " + resource + "\n"]
        else:
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
        message = self.xor_encrypt(self.team_name, bytes.fromhex(message[11:]).decode("utf-8"))
        if "inventory" in message:
            self.update_team_backpack(message[9:])
        if "incantation" in message:
            if self.clear_message_flag == 1:
                self.clear_message_flag = 0
                return
            if self.ritual_leader >= 1 and int(message.split(";")[0]) > self.bot_id:
                self.ritual_leader = 0
                self.ritual_mode = 0
                self.state = 0
                return
            if self.state > -1 and self.state < 4 and self.backpack["food"] > 35:
                self.state = 4
                self.action_queue = []
            if self.ritual_mode == 1:
                self.action_queue = self.move_to_message_source(direction)
        if "on my way" in message and self.ritual_leader >= 1:
            self.players_for_ritual += 1
        if "ready" in message and self.ritual_leader >= 1:
            self.ritual_leader += 1

    def move_to_message_source(self, direction: int) -> list:
        if self.ritual_ready == 1 or self.action_queue:
            return []  # Return empty list instead of None
        actions = []
        if (direction == 0):
            message = bytes(self.xor_encrypt(self.team_name, ("ready")), "utf-8").hex()
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
        if self.action_queue:
            return
        data = self.vision.split(",")[0]
        while True:
            if len(data) == 0 or data[0].isalpha():
                break
            data = data[1:]
        data = data.split(" ")
        needed_materials = LEVEL_REQUIREMENTS[self.level].copy()
        for k in needed_materials:
            for j in data:
                if j == k:
                    needed_materials[k] -= 1
        for k in needed_materials:
            if needed_materials[k] < 1:
                continue
            if k in self.backpack and self.backpack[k] != 0:
                self.action_queue = ["Set " + k + "\n"]
                self.action_queue.append("Look\n")
                self.backpack[k] -= 1
                return
        self.state = 7
        self.action = ""
        return

    def begin_ritual(self):
        data = self.vision.split(",")[0]
        while True:
            if len(data) == 0 or data[0].isalpha():
                break
            data = data[1:]
        data = data.split(" ")
        needed_materials = LEVEL_REQUIREMENTS[self.level].copy()
        for k in needed_materials:
            for j in data:
                if j == k:
                    needed_materials[k] -= 1
        for k in needed_materials:
            if needed_materials[k] > 0:
                return
        self.action = "Incantation\n"
        self.action_queue = ["Incantation\n"]
        self.state += 1

    def decide_action(self):
        if self.state == -2:
            self.action = "Connect_nbr\n"
            self.state += 1
        elif self.state == -1:
            if self.bot_id < 6 and self.free_slots == 0:
                self.action = "Fork\n"
                self.reproduction = 1
            else:
                self.action = "Look\n"
            self.state += 1
        elif self.state == 0:
            if self.action_queue:
                self.action = self.action_queue[0]
                self.action_queue = self.action_queue[1:]
            else:
                self.action = "Inventory\n"
                self.state += 1
        elif self.state == 1:
            if self.found_new_item:
                if not self.can_perform_ritual():
                    message = bytes(self.xor_encrypt(self.team_name, ("inventory" + str(self.bot_id) + ";" + str(self.level) + ";" + str(json.dumps(self.backpack)))), "utf-8").hex()
                    self.action = "Broadcast " + message + "\n"
                else:
                    message = bytes(self.xor_encrypt(self.team_name, (str(self.bot_id) + ";incantation;" + str(self.level))), "utf-8").hex()
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
            if self.ritual_mode == 0:
                data = bytes(self.xor_encrypt(self.team_name, str(self.bot_id) + " on my way"), "utf-8").hex()
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
            self.clear_message_flag = 0
            if self.ritual_leader >= 6:
                self.begin_ritual()
            if self.state != 7:
                self.place_resources_for_ritual()
                if self.action_queue:
                    self.action = self.action_queue[0]
                    self.action_queue = self.action_queue[1:]
                else:
                    self.action = "Inventory\n"
        elif self.state == 7:
            if self.ritual_leader < 6:
                self.action = "Connect_nbr\n"
                return
            if self.action_queue:
                    self.action = self.action_queue[0]
                    self.action_queue = self.action_queue[1:]
            else:
                self.action_queue = ["Inventory\n"]
                self.action_queue = ["Look\n"]
        elif self.state == 8:
            self.action = ""
        elif self.state == 9:
            self.action = "Inventory\n"
            self.state += 1
        elif self.state == 10:
            message = bytes(self.xor_encrypt(self.team_name, ("inventory" + str(self.bot_id) + ";" + str(self.level) + ";" + str(json.dumps(self.backpack)))), "utf-8").hex()
            self.action = "Broadcast " + message + "\n"
            self.state = 0
