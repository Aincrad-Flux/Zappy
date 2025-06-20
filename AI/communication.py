#!/usr/bin/env python3

"""
Communication module for the Zappy AI
"""

import socket
import re
import queue
import threading
import time
import os
import sys
from utils import encrypt_message, decrypt_message

class CommunicationManager:
    """Manages communication with the Zappy server"""

    def __init__(self, socket, client_num, team_name, logger):
        """Initialize the communication manager"""
        self.socket = socket
        self.client_num = client_num
        self.team_name = team_name
        self.logger = logger
        self.running = True
        self.response_queue = queue.Queue()
        self.encryption_key = f"{team_name}_key"  # Simple key for team

        # Team coordination
        self.team_members = set()
        self.team_member_positions = {}  # Store teammate positions
        self.incantation_participants = {}  # Players who've responded to elevation

        # Beacon system
        self.active_beacons = {}  # Dictionary of active beacons by level
        self.beacon_last_update = {}  # Last time a beacon was heard from

        # Broadcasting configuration
        self.last_broadcast_time = 0
        self.broadcast_cooldown = 2  # Seconds between broadcasts

        # Broadcast message tracking
        self.last_raw_message = ""
        self.last_decoded_message = ""
        self.last_broadcast_direction = None
        self.last_broadcast_time = time.time()

        # Terminal UI state
        self.is_beacon = False
        self.beacon_mode = ""
        self.inventory = {}
        self.current_state = "COLLECTING_RESOURCES"  # Current AI state
        self.elevation_status = {"status": "UNKNOWN", "message": "Elevation status unknown"}
        self.override_message = ""
        self.override_beacon_id = None
        self.override_level = None
        self.override_time = 0
        self.override_duration = 5  # Seconds to display the override message
        self.last_ui_update = 0
        self.ui_update_interval = 0.5  # Update the UI every 0.5 seconds

        # Vision cone display
        self.last_vision_tiles = []  # Store the last vision data
        self.vision_level = 1  # Current vision level

        # Start the UI update thread
        self.ui_thread = threading.Thread(target=self._update_terminal_ui)
        self.ui_thread.daemon = True
        self.ui_thread.start()

        # Start the receiver thread
        self.receiver_thread = threading.Thread(target=self._response_receiver)
        self.receiver_thread.daemon = True
        self.receiver_thread.start()

    def send_command(self, command):
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

                            # Process other message parts in the _process_team_message method
                            self._process_team_message(decrypted_content, direction)

                except Exception as e:
                    self.logger.warning(f"Failed to decrypt message: {e}")
                    return
            else:
                # Otherwise, just use the content as is (might be from another team)
                decrypted_content = content
                self.logger.info(f"Processing unencrypted message: {decrypted_content}")

                # Process if it's a team message
                if decrypted_content.startswith("TEAM:"):
                    self._process_team_message(decrypted_content, direction)

    def _process_team_message(self, content, direction):
        """Process a team message"""
        parts = content.split(":")
        if len(parts) >= 3:
            sender_id = parts[1]
            message_type = parts[2]

            self.logger.info(f"Team message from {sender_id}, type: {message_type}")

            # Update sender's estimated position based on sound direction
            self.team_member_positions[sender_id] = {
                "direction": direction,
                "timestamp": time.time()
            }

            if message_type == "BEACON":
                # Someone is broadcasting as a beacon for elevation
                level = int(parts[3]) if len(parts) > 3 else 1

                self.logger.info(f"Received BEACON signal from {sender_id} for level {level}")

                # Update or add to active beacons
                self.active_beacons[level] = sender_id
                self.beacon_last_update[sender_id] = time.time()

                # Show override message in terminal UI
                self.show_override_message(sender_id, level)

                # Record direction to beacon
                self.team_member_positions[sender_id] = {
                    "direction": direction,
                    "timestamp": time.time(),
                    "is_beacon": True,
                    "level": level
                }

            elif message_type == "BEACON_RESPONSE":
                # Someone is responding to a beacon
                beacon_id = parts[3] if len(parts) > 3 else None
                level = int(parts[4]) if len(parts) > 4 else 1

                if beacon_id and beacon_id == str(self.client_num):
                    self.logger.info(f"Received BEACON_RESPONSE from {sender_id} for level {level}")

                    # Record the participant
                    self.incantation_participants[sender_id] = {
                        "type": "BEACON_RESPONSE",
                        "level": level,
                        "timestamp": time.time(),
                        "direction": direction
                    }

            elif message_type == "ELEVATION":
                # Someone is asking for elevation
                level = int(parts[3]) if len(parts) > 3 else 1
                resources_data = parts[4] if len(parts) > 4 else ""

                self.logger.info(f"Received elevation request from {sender_id} for level {level}")
                self.incantation_participants[sender_id] = {
                    "type": "ELEVATION",
                    "level": level,
                    "resources": resources_data,
                    "direction": direction,
                    "timestamp": time.time()
                }

            elif message_type == "ELEVATION_RESPONSE":
                # Someone is responding to our elevation request
                level = int(parts[3]) if len(parts) > 3 else 1

                self.logger.info(f"Received elevation response from {sender_id} for level {level}")
                self.incantation_participants[sender_id] = {
                    "type": "ELEVATION_RESPONSE",
                    "level": level,
                    "timestamp": time.time()
                }

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
                self.logger.info(f"Team member {sender_id} needs {resource_type}")

    def broadcast(self, text):
        """Broadcast a message to teammates"""
        # Check if we need to encrypt the message first
        if text.startswith("TEAM:"):
            encrypted = encrypt_message(text, self.encryption_key)
            final_text = f"ENC:{encrypted}"
        else:
            final_text = text

        response = self.send_command(f"Broadcast {final_text}")
        return response == "ok"

    def broadcast_elevation_request(self, level):
        """Broadcast an elevation request to teammates"""
        message = f"TEAM:{self.client_num}:ELEVATION:{level}:unknown"
        self.broadcast(message)

    def broadcast_elevation_response(self, request_id, level):
        """Broadcast a response to an elevation request"""
        message = f"TEAM:{self.client_num}:ELEVATION_RESPONSE:{level}:{request_id}"
        self.broadcast(message)

    def broadcast_resources(self, resources_data):
        """Broadcast resource information"""
        message = f"TEAM:{self.client_num}:RESOURCES:{resources_data}"
        self.broadcast(message)

    def broadcast_level_up(self, level):
        """Broadcast a level up notification"""
        message = f"TEAM:{self.client_num}:LEVEL_UP:{level}"
        self.broadcast(message)

    def broadcast_beacon(self, level):
        """Broadcast a beacon signal for players to join for evolution"""
        message = f"TEAM:{self.client_num}:BEACON:{level}"
        self.broadcast(message)

    def broadcast_beacon_response(self, beacon_id, level):
        """Broadcast a response to a beacon signal"""
        message = f"TEAM:{self.client_num}:BEACON_RESPONSE:{beacon_id}:{level}"
        self.broadcast(message)

    def get_active_beacon(self, level=None):
        """Get active beacon for a specific level if provided, otherwise any beacon"""
        current_time = time.time()
        beacon_timeout = 10  # Consider beacons older than 10 seconds expired

        if level and level in self.active_beacons:
            beacon_id = self.active_beacons[level]
            last_update = self.beacon_last_update.get(beacon_id, 0)

            if current_time - last_update < beacon_timeout:
                # Show override message for a newly detected beacon
                if beacon_id != self.override_beacon_id or level != self.override_level:
                    self.show_override_message(beacon_id, level)
                return beacon_id

        # If level not specified or no beacon for that level, check for any beacon
        for l, beacon_id in self.active_beacons.items():
            last_update = self.beacon_last_update.get(beacon_id, 0)
            if current_time - last_update < beacon_timeout:
                if level is None or l == level:
                    # Show override message for a newly detected beacon
                    if beacon_id != self.override_beacon_id or l != self.override_level:
                        self.show_override_message(beacon_id, l)
                    return beacon_id

        return None

    def cleanup(self):
        """Cleanup resources"""
        self.running = False
        if hasattr(self, 'receiver_thread') and self.receiver_thread.is_alive():
            self.receiver_thread.join(timeout=1)
        if hasattr(self, 'ui_thread') and self.ui_thread.is_alive():
            self.ui_thread.join(timeout=1)

    def _update_terminal_ui(self):
        """Update the terminal UI with player info"""
        while self.running:
            try:
                current_time = time.time()

                if current_time - self.last_ui_update >= self.ui_update_interval:
                    # Clear screen - works on both Unix/Linux and Windows
                    os.system('cls' if os.name == 'nt' else 'clear')

                    # Print header
                    print(f"{'='*60}")
                    print(f"ZAPPY AI #{self.client_num} - TEAM: {self.team_name}")
                    print(f"{'='*60}")

                    # Print beacon status
                    if self.is_beacon:
                        print(f"\n[BEACON MODE: {self.beacon_mode}]")
                    else:
                        # Check if we're following a beacon
                        following_beacon_id = self.override_beacon_id
                        if following_beacon_id:
                            # Display direction information if available
                            direction = self.get_beacon_direction(following_beacon_id)
                            if direction is not None:
                                direction_labels = {
                                    1: "Front", 2: "Front-Left", 3: "Left",
                                    4: "Back-Left", 5: "Back", 6: "Back-Right",
                                    7: "Right", 8: "Front-Right", 0: "Same Tile"
                                }
                                direction_name = direction_labels.get(direction, f"Unknown({direction})")
                                print(f"\n[FOLLOWING BEACON #{following_beacon_id} - Direction: {direction_name} ({direction})]")
                            else:
                                print(f"\n[FOLLOWING BEACON #{following_beacon_id}]")
                        else:
                            # Display current state instead of just NORMAL MODE
                            print(f"\n[STATE: {self.current_state}]")

                    # Print inventory
                    print("\nINVENTORY:")
                    print("-" * 30)
                    for resource, amount in self.inventory.items():
                        print(f"{resource}: {amount}")

                    # Print elevation status
                    print("\nELEVATION STATUS:")
                    print("-" * 30)

                    # Colorize the status message
                    RESET = "\033[0m"
                    GREEN = "\033[1;32m"  # Bright Green
                    YELLOW = "\033[1;33m"  # Bright Yellow
                    RED = "\033[1;31m"     # Bright Red
                    CYAN = "\033[1;36m"    # Bright Cyan

                    status = self.elevation_status.get("status", "UNKNOWN")
                    message = self.elevation_status.get("message", "Elevation status unknown")
                    missing = self.elevation_status.get("missing", [])

                    if status == "READY":
                        print(f"{GREEN}✓ {message}{RESET}")
                    elif status == "BEACON":
                        print(f"{CYAN}◉ {message}{RESET}")
                    elif status == "MAX_LEVEL":
                        print(f"{GREEN}★ {message}{RESET}")
                    elif status == "MISSING":
                        print(f"{YELLOW}⚠ {message}{RESET}")
                        # Print missing items with detailed information
                        if missing:
                            print(f"  {RED}Missing requirements:{RESET}")
                            for item in missing:
                                print(f"  {YELLOW}• {item}{RESET}")
                    else:
                        print(f"{YELLOW}? {message}{RESET}")

                    # Print vision cone
                    vision_display = self._generate_vision_display()
                    print(vision_display)

                    # Print override message if active
                    if self.override_message and (current_time - self.override_time < self.override_duration):
                        print("\n" + "!" * 60)
                        print(self.override_message)
                        print("!" * 60)

                    # Print vision cone details if available
                    if self.last_vision_tiles:
                        print("\nVISION DETAILS:")
                        print("-" * 30)
                        for i, tile in enumerate(self.last_vision_tiles):
                            if i < 5:  # Limit to first 5 tiles to avoid cluttering the UI
                                print(f"Tile {i}: {tile}")
                        if len(self.last_vision_tiles) > 5:
                            print(f"... and {len(self.last_vision_tiles) - 5} more tiles")

                    # Print last broadcast message if available
                    if self.last_decoded_message:
                        print(f"\nLAST BROADCAST MESSAGE: {self.last_decoded_message}")
                        if self.last_broadcast_direction is not None:
                            direction_labels = {
                                1: "Front", 2: "Front-Left", 3: "Left",
                                4: "Back-Left", 5: "Back", 6: "Back-Right",
                                7: "Right", 8: "Front-Right", 0: "Same Tile"
                            }
                            direction_name = direction_labels.get(self.last_broadcast_direction, f"Unknown({self.last_broadcast_direction})")
                            print(f"DIRECTION: {direction_name} ({self.last_broadcast_direction})")

                    sys.stdout.flush()
                    self.last_ui_update = current_time

                time.sleep(0.1)

            except Exception as e:
                self.logger.error(f"Error updating terminal UI: {e}")
                time.sleep(1)

    def update_inventory(self, inventory):
        """Update the UI with current inventory"""
        self.inventory = inventory

    def set_beacon_mode(self, is_beacon, level=""):
        """Update the UI with current beacon status"""
        self.is_beacon = is_beacon
        self.beacon_mode = f"Level {level}" if level else ""

    def show_override_message(self, beacon_id, level):
        """Show beacon override message"""
        # Get direction if available
        direction = self.get_beacon_direction(beacon_id)
        direction_info = ""
        if direction is not None:
            direction_labels = ["Behind", "Behind-Right", "Right", "Front-Right", "Front", "Front-Left", "Left", "Behind-Left"]
            direction_info = f" - Direction: {direction_labels[direction]} ({direction})"

        self.override_message = f"BEACON DETECTED! Player #{beacon_id} is signaling for Level {level} elevation!{direction_info}"
        self.override_beacon_id = beacon_id
        self.override_level = level
        self.override_time = time.time()

    def clear_override_message(self):
        """Clear the override message"""
        self.override_message = ""
        self.override_beacon_id = None

    def get_beacon_direction(self, beacon_id):
        """Get the direction to a specific beacon"""
        if beacon_id in self.team_member_positions:
            position = self.team_member_positions[beacon_id]
            if 'direction' in position and time.time() - position['timestamp'] < 5:
                return position['direction']
        return None

    def get_team_member_info(self):
        """Get information about team members"""
        return {
            'positions': self.team_member_positions,
            'participants': self.incantation_participants
        }

    def update_vision_data(self, vision_tiles, vision_level):
        """Update the vision data for the terminal UI"""
        self.last_vision_tiles = vision_tiles
        self.vision_level = vision_level

    def update_broadcast_info(self, raw_message, decoded_message, direction):
        """Update broadcast message information for display"""
        self.last_raw_message = raw_message
        self.last_decoded_message = decoded_message
        self.last_broadcast_direction = direction
        self.last_broadcast_time = time.time()

    def get_last_raw_message(self):
        """Get the last raw broadcast message received"""
        return self.last_raw_message

    def get_last_decoded_message(self):
        """Get the last decoded broadcast message"""
        return self.last_decoded_message

    def update_state(self, state):
        """Update the current state of the AI"""
        self.current_state = state

    def update_elevation_status(self, status):
        """Update the UI with current elevation status"""
        self.elevation_status = status

    def _generate_vision_display(self):
        """Generate a string representation of the vision cone"""
        if not self.last_vision_tiles:
            return "\nVISION CONE:\n" + "-" * 30 + "\nNo vision data available"

        # ANSI color codes for better visibility
        RESET = "\033[0m"
        PLAYER_COLOR = "\033[1;36m"  # Bright Cyan
        FOOD_COLOR = "\033[1;32m"    # Bright Green
        RESOURCE_COLOR = "\033[1;33m" # Bright Yellow
        EMPTY_COLOR = "\033[0;90m"   # Dark Gray

        # Create a string representation of the vision cone
        display = []
        display.append("\nVISION CONE:")
        display.append("-" * 30)

        # Add legend with colors
        display.append(f"Legend: {PLAYER_COLOR}p{RESET} - player, {FOOD_COLOR}f{RESET} - food, {RESOURCE_COLOR}*{RESET} - resource")

        # In Zappy, vision works differently:
        # Level 1: [current tile + 3 in front = 4 tiles]
        # Level 2: [current tile + 5 in front + 3 more = 9 tiles]
        # Level n: n+(n+1)+(n+2)+...+(n+(n-1)) = n^2 tiles

        # We need to organize these tiles into a diamond/triangle shape
        # For the example of level 2 vision:
        #     .
        #    . .
        #   . . .
        #    . .
        #     p

        level = self.vision_level
        if level < 1:
            level = 1

        # Calculate number of rows and layout
        rows = 2 * level - 1
        middle_row = level - 1  # 0-indexed

        # Start with the player's position (last tile)
        player_tile_index = 0

        # For each row, calculate the number of tiles and their indices
        tile_index = 0

        # Process each row of the vision cone (from top to bottom)
        for row_idx in range(rows):
            # Calculate how far we are from the middle row (0 for middle, positive for both directions)
            distance_from_middle = abs(row_idx - middle_row)

            # Calculate row width (widest at middle_row, narrower at edges)
            row_width = 2 * (level - distance_from_middle) - 1

            # Leading spaces for centering
            leading_spaces = " " * distance_from_middle

            row_str = leading_spaces

            # Process each tile in the current row
            for j in range(row_width):
                if tile_index < len(self.last_vision_tiles):
                    tile_content = self.last_vision_tiles[tile_index]

                    # Create a visual representation of the tile
                    has_player = False
                    has_food = False
                    has_resource = False

                    for item in tile_content:
                        if item == "player":
                            has_player = True
                        elif item == "food":
                            has_food = True
                        elif item in ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]:
                            has_resource = True

                    # If there's nothing in the tile, show a dot
                    if not (has_player or has_food or has_resource):
                        row_str += f"{EMPTY_COLOR}.{RESET} "
                    else:
                        # Prioritize player, then food, then resources
                        if has_player:
                            row_str += f"{PLAYER_COLOR}p{RESET} "
                        elif has_food:
                            row_str += f"{FOOD_COLOR}f{RESET} "
                        elif has_resource:
                            row_str += f"{RESOURCE_COLOR}*{RESET} "

                    tile_index += 1
                else:
                    row_str += f"{EMPTY_COLOR}.{RESET} "  # Fill with empty tiles if we run out

            display.append(row_str)

        # Return the complete vision display as a string
        return "\n".join(display)
