#!/usr/bin/env python3

"""
Communication module for the Zappy AI
"""

import socket
import re
import queue
import threading
import time
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

        # Broadcasting configuration
        self.last_broadcast_time = 0
        self.broadcast_cooldown = 2  # Seconds between broadcasts

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

            if message_type == "ELEVATION":
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

    def get_team_member_info(self):
        """Get information about team members"""
        return {
            "positions": self.team_member_positions,
            "participants": self.incantation_participants
        }

    def cleanup(self):
        """Cleanup resources and stop threads"""
        self.running = False
