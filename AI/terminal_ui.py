#!/usr/bin/env python3
##
## EPITECH PROJECT, 2025
## Zappy
## File description:
## Terminal user interface for the AI client
##

import os
import sys
import time
import threading
import curses

from AI.utils import format_inventory
from AI.logger import get_logger

class TerminalUI:
    """
    A simple terminal UI for the Zappy AI client.

    This class implements a basic terminal user interface that displays
    information about the AI's status, inventory, and level requirements.
    It runs in a separate thread to avoid blocking the main AI logic.
    """

    def __init__(self, agent, network_client, refresh_rate=1):
        """
        Initialize the TerminalUI with references to the AI agent.

        Args:
            agent: Reference to the AICore instance
            network_client: Reference to the NetworkClient instance
            refresh_rate (float): UI refresh rate in seconds
        """
        self.agent = agent
        self.client = network_client
        self.refresh_rate = refresh_rate
        self.running = False
        self.thread = None
        self.logger = get_logger(bot_id=self.agent.bot_id, team_name=self.agent.team_name)
        self.logger.info("Terminal UI initialized")

    def start(self):
        """Start the UI thread if it's not already running."""
        if not self.running:
            self.running = True
            self.thread = threading.Thread(target=self._run_ui_loop)
            self.thread.daemon = True
            self.thread.start()
            self.logger.info("Terminal UI started")

    def stop(self):
        """Stop the UI thread if it's running."""
        self.running = False
        if self.thread:
            self.thread.join(timeout=2)
            self.thread = None
            self.logger.info("Terminal UI stopped")

    def _run_ui_loop(self):
        """Main UI loop running in a separate thread."""
        stdscr = None
        try:
            stdscr = curses.initscr()
            curses.start_color()
            curses.use_default_colors()
            curses.init_pair(1, curses.COLOR_GREEN, -1)  # Green for good
            curses.init_pair(2, curses.COLOR_RED, -1)    # Red for bad
            curses.init_pair(3, curses.COLOR_YELLOW, -1) # Yellow for info
            curses.init_pair(4, curses.COLOR_CYAN, -1)   # Cyan for headers
            curses.noecho()
            curses.cbreak()
            stdscr.keypad(True)
            stdscr.nodelay(True)
            stdscr.clear()

            while self.running:
                height, width = stdscr.getmaxyx()
                stdscr.clear()
                title = f" ZAPPY AI - Team: {self.agent.team_name} | Bot ID: {self.agent.bot_id} "
                stdscr.addstr(0, (width - len(title)) // 2, title, curses.A_BOLD)

                connected_str = "CONNECTED" if self.client.connected else "DISCONNECTED"
                connected_style = curses.color_pair(1) if self.client.connected else curses.color_pair(2)
                stdscr.addstr(1, 2, f"Status: ", curses.A_BOLD)
                stdscr.addstr(connected_str, connected_style | curses.A_BOLD)

                stdscr.addstr(3, 2, "Bot Information", curses.color_pair(4) | curses.A_BOLD)
                stdscr.addstr(4, 4, f"Level: {self.agent.level}", curses.A_BOLD)
                stdscr.addstr(5, 4, f"State: {self._get_state_description()}")
                stdscr.addstr(6, 4, f"Current action: {self.agent.action}")
                stdscr.addstr(7, 4, f"Players needed for ritual: {self._get_players_needed()}")

                stdscr.addstr(9, 2, "Inventory", curses.color_pair(4) | curses.A_BOLD)
                inventory_str = format_inventory(self.agent.backpack)
                inventory_lines = self._wrap_text(inventory_str, width - 6)
                for i, line in enumerate(inventory_lines):
                    stdscr.addstr(10 + i, 4, line)

                next_level_reqs = self._get_level_requirements()
                stdscr.addstr(13, 2, "Elevation Requirements", curses.color_pair(4) | curses.A_BOLD)
                stdscr.addstr(14, 4, next_level_reqs)
                missing_resources = self._get_missing_resources()
                stdscr.addstr(16, 2, "Missing Resources", curses.color_pair(4) | curses.A_BOLD)
                stdscr.addstr(17, 4, missing_resources)

                if "total" in self.agent.team_backpack:
                    stdscr.addstr(19, 2, "Team Resources (Total)", curses.color_pair(4) | curses.A_BOLD)
                    team_inv_str = format_inventory(self.agent.team_backpack["total"])
                    team_inv_lines = self._wrap_text(team_inv_str, width - 6)
                    for i, line in enumerate(team_inv_lines):
                        stdscr.addstr(20 + i, 4, line)

                stdscr.addstr(height - 2, 2, "Press 'q' to quit UI mode", curses.color_pair(3) | curses.A_BOLD)

                try:
                    key = stdscr.getch()
                    if key == ord('q'):
                        break
                except:
                    pass

                stdscr.refresh()
                time.sleep(self.refresh_rate)

        except Exception as e:
            self.logger.error(f"UI Error: {str(e)}")
        finally:
            if stdscr:
                curses.nocbreak()
                stdscr.keypad(False)
                curses.echo()
                curses.endwin()
            self.running = False

    def _get_state_description(self):
        """Get a human-readable description of the agent's state."""
        state_map = {
            -2: "Initializing",
            -1: "Connecting",
            0: "Idle",
            1: "Exploring",
            2: "Collecting resources",
            3: "Ritual preparation",
            4: "Ritual in progress",
            5: "Reproduction",
        }
        return state_map.get(self.agent.state, f"Unknown ({self.agent.state})")

    def _get_level_requirements(self):
        """Get a formatted string of requirements for the next level."""
        from AI.ai_core import LEVEL_REQUIREMENTS

        if self.agent.level >= 8:
            return "Maximum level reached!"

        requirements = LEVEL_REQUIREMENTS.get(self.agent.level, {})
        if not requirements:
            return "No requirements"

        req_str = []
        for resource, amount in requirements.items():
            req_str.append(f"{resource}: {amount}")

        return ", ".join(req_str)

    def _get_missing_resources(self):
        """Calculate and return a string of missing resources for level up."""
        from AI.ai_core import LEVEL_REQUIREMENTS

        if self.agent.level >= 8:
            return "Maximum level reached!"

        requirements = LEVEL_REQUIREMENTS.get(self.agent.level, {})
        if not requirements:
            return "No requirements"

        missing = {}
        for resource, amount in requirements.items():
            current = self.agent.backpack.get(resource, 0)
            if current < amount:
                missing[resource] = amount - current

        if not missing:
            return "All resources collected!"

        missing_str = []
        for resource, amount in missing.items():
            missing_str.append(f"{resource}: {amount}")

        return ", ".join(missing_str)

    def _get_players_needed(self):
        """Calculate players needed for ritual at current level."""
        return self.agent.level + 1

    def _wrap_text(self, text, width):
        """Wrap text to fit within a specified width."""
        lines = []
        while len(text) > width:
            space_pos = text[:width].rfind(' ')
            if space_pos == -1:
                lines.append(text[:width])
                text = text[width:]
            else:
                lines.append(text[:space_pos])
                text = text[space_pos+1:]

        if text:
            lines.append(text)

        return lines
