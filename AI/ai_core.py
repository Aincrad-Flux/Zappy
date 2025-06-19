#!/usr/bin/env python3

"""
Core AI module for the Zappy game
"""

import time
import random
from models import ResourceType
from utils import setup_logger
from communication import CommunicationManager
from actions import ActionManager
from strategy import StrategyManager

class ZappyAI:
    """Implementation of the Zappy AI"""

    def __init__(self, socket, client_num, width, height, team_name=""):
        """Initialize the AI"""
        # Setup logger
        self.logger = setup_logger(client_num, team_name)
        self.logger.info(f"AI client #{client_num} for team {team_name} initialized")
        self.logger.info(f"Map dimensions: {width}x{height}")

        # Game parameters
        self.client_num = client_num
        self.width = width
        self.height = height
        self.team_name = team_name
        self.running = True

        # Initialize the subsystems
        self.comm_manager = CommunicationManager(socket, client_num, team_name, self.logger)
        self.action_manager = ActionManager(self.comm_manager, self.logger)
        self.strategy_manager = StrategyManager(self.action_manager, self.comm_manager, self.logger)

    def run(self):
        """Main AI loop"""
        last_fork_time = 0
        fork_cooldown = 100  # Time between fork attempts

        try:
            self.logger.info("Starting AI main loop")

            while self.running:
                # Decide what to do next
                self.strategy_manager.decide_next_action()

                # Broadcast status to teammates occasionally
                self.strategy_manager.broadcast_status()

                # Try to fork new players periodically
                current_time = time.time()
                inventory = self.action_manager.get_inventory()

                if (current_time - last_fork_time > fork_cooldown and
                    inventory[ResourceType.FOOD] > 20):  # Only fork if we have enough food
                    slots = self.action_manager.connect_nbr()
                    if slots > 0:
                        self.logger.info(f"Attempting to fork, {slots} slots available")
                        self.action_manager.fork()
                        last_fork_time = current_time

                # Execute action based on current state
                self.strategy_manager.execute_current_state()

                # Sleep to avoid overwhelming the server
                time.sleep(0.1)

        except Exception as e:
            self.logger.error(f"Error in AI loop: {e}")
        finally:
            self.logger.info("AI shutting down")
            self.running = False
            self.comm_manager.cleanup()

    def stop(self):
        """Stop the AI"""
        self.running = False
