#!/usr/bin/env python3
##
## EPITECH PROJECT, 2025
## Zappy
## File description:
## Launcher script for the AI client (Python version)
##

import sys
import os

current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.dirname(current_dir)
sys.path.insert(0, parent_dir)

if __name__ == "__main__":
    from AI.main import main

    try:
        hostname, port, name, bot_id, use_ui = main()

        from AI.network_client import NetworkClient

        client = NetworkClient(hostname, port, name, bot_id, use_ui)
        client.establish_connection()

        ui = None
        if use_ui:
            from AI.terminal_ui import TerminalUI
            ui = TerminalUI(client.agent, client)
            ui.start()

        try:
            client.run_client()
        finally:
            client.check_server_capacity()
            client.disconnect()

            if ui:
                ui.stop()
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

