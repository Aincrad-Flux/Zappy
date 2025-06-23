from http import client
from re import I
import socket
import os
from AI.utils import safe_convert
from AI.ai_core import AICore
import logging
import selectors
import random
import subprocess

class NetworkClient():
    """This class implements a client which use asyncio for non-blocking communication
    We could connect, interact with data, and interact with the server

    Args:
        socket (socket.socket): the socket which allows to communicate with the server
    """

    def __init__(self, hostname: str, port: str, team_name: str, bot_id: str):
        """NetworkClient constructor

        Args:
            hostname (str): ip address of the server
            port (str): port of the server
            team_name (str): name of the team to connect
        """
        self.team = team_name
        self.hostname = hostname
        self.port = port
        self.bot_id = int(bot_id)
        self.map_width = 0
        self.map_height = 0
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Initialize socket right away
        self.selector = selectors.DefaultSelector()
        self.init_status = 0
        self.connected = 0
        self.agent = AICore(self.team)
        self.agent.bot_id = self.bot_id
        self.counter = 0

    def establish_connection(self):
        """Initialise a socket which connects to the server
        We initialize the selectors to and be sure to receive the Welcome message
        """
        # Socket is already initialized in __init__
        self.socket.setblocking(False)
        port_num = safe_convert(self.port, int)
        if port_num is None:
            port_num = 4242  # Default port if conversion fails
        self.socket.connect_ex((self.hostname, port_num))
        events = selectors.EVENT_READ | selectors.EVENT_WRITE
        self.selector.register(self.socket, events)

    def handle_server_info(self, message, stage):
        """save start info
        """
        if stage == 1:
            slots = safe_convert(message, int)
            self.agent.free_slots = 0 if slots is None else slots
            self.agent.action = ""
            self.init_status = 2
        else:
            message = message.split()
            width = safe_convert(message[0], int)
            height = safe_convert(message[1], int)
            self.map_width = 0 if width is None else width
            self.map_height = 0 if height is None else height
            self.agent.action = ""
            self.init_status = 3
            self.connected = 1
            self.agent.active = 1


    def run_client(self):
        """Process the init data delivers by the server
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
                            print("Connection closed by the server")
                            self.selector.unregister(self.socket)
                            self.socket.close()
                            break  # Exit the loop
                    except Exception as e:
                        print(f"Error receiving data: {e}")
                        self.selector.unregister(self.socket)
                        self.socket.close()
                        break  # Exit the loop
                    tmp = message.split("\n")
                    for elem in tmp[:-1]:
                        if "dead" in elem:
                            print("Bot died, id: ", self.bot_id)
                            exit(0)
                        if "WELCOME" in elem and self.connected == 0:
                            self.agent.action = (self.team + '\n')
                        elif self.init_status < 3:
                            if "message" in elem:
                                continue
                            self.handle_server_info(elem, self.init_status)
                        elif  "Elevation underway" in elem:
                            self.agent.state = 8
                        elif "Current level:" in elem:
                            self.agent.level = int(''.join(filter(str.isdigit, elem)))
                            if self.agent.level == 8:
                                exit(0)
                            print(elem)
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
                            # Store as single message instead of appending to list
                            self.agent.message_received = [elem]
                            self.agent.handle_message(elem)
                            message = message.split("\n")[-1]
                            continue
                        elif isinstance(self.agent.action, str) and self.agent.action.startswith("Take ") and "food" not in self.agent.action and elem == "ok":
                            self.agent.refresh_team_inventory()
                            self.agent.found_new_item = True
                        elif self.agent.action == "Inventory\n":
                            try:
                                self.agent.parse_backpack(elem)
                            except ValueError:
                                print("Error ", elem)
                                pass
                        elif self.agent.action == "Look\n":
                            self.agent.vision = elem
                        elif self.agent.action == "Connect_nbr\n":
                            try:
                                slots = int(elem)
                                self.agent.free_slots = slots
                                if slots > 0 and self.bot_id < 6 and self.agent.reproduction == 1:
                                    script_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "zappy_ai")
                                    subprocess.Popen(["python3", script_path, "-p", self.port, "-n", self.team, "-i", str(self.bot_id + 1)])
                                    print("Forking new bot ", self.agent.bot_id)
                                    self.agent.reproduction = 0
                            except ValueError:
                                print("Error parsing Connect_nbr response:", elem)
                                self.agent.free_slots = 0
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
                            if isinstance(self.agent.action, str):  # Make sure action is a string before encoding
                                self.socket.send(self.agent.action.encode())
                            else:
                                print(f"Warning: agent.action is not a string: {self.agent.action}")
                        except Exception as e:
                            print(f"Error sending message: {e}")
                        self.agent.active = 0

    def disconnect(self):
        """close the client
        """
        try:
            self.selector.unregister(self.socket)
            self.socket.close()
        except Exception as e:
            # Just log and continue if there's an issue closing the connection
            logging.error(f"Error when disconnecting: {e}")

    def check_server_capacity(self):
        """Check if the server have a slot available, else shutdown the client
        """
        if self.bot_id < 0:
            logging.info("The server does not have available slots")
