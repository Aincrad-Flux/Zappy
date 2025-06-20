#!/usr/bin/env python3
import tkinter as tk
from tkinter import ttk, scrolledtext
import subprocess
import os
import sys
import shlex

class ZappyLauncher:
    def __init__(self, root):
        self.root = root
        self.root.title("Zappy Launcher")
        self.root.geometry("800x600")

        # Create tabs
        self.notebook = ttk.Notebook(root)
        self.tab_config = ttk.Frame(self.notebook)
        self.tab_logs = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_config, text="Configuration")
        self.notebook.add(self.tab_logs, text="Logs")
        self.notebook.pack(expand=1, fill="both")

        # Variables
        self.port = tk.StringVar(value="1234")
        self.host = tk.StringVar(value="127.0.0.1")
        self.map_width = tk.StringVar(value="20")
        self.map_height = tk.StringVar(value="20")
        self.client_limit = tk.StringVar(value="5")
        self.freq = tk.StringVar(value="25")
        self.match_duration = tk.StringVar(value="600")
        self.auto_start = tk.StringVar(value="on")
        self.display_eggs = tk.StringVar(value="true")
        self.gui_mode = tk.StringVar(value="2d")
        self.ai_terminal_ui = tk.BooleanVar(value=True)

        # Team management
        self.teams = ["team1", "team2", "team3"]
        self.team_vars = {}
        self.ai_count_vars = {}

        # Server process
        self.server_process = None
        self.gui_process = None
        self.ai_processes = []

        # UI creation
        self._create_config_tab()
        self._create_logs_tab()

    def _create_config_tab(self):
        # Create a frame with scrollbar for the config tab
        config_frame = ttk.Frame(self.tab_config)
        config_frame.pack(fill="both", expand=True)

        canvas = tk.Canvas(config_frame)
        scrollbar = ttk.Scrollbar(config_frame, orient="vertical", command=canvas.yview)
        scrollable_frame = ttk.Frame(canvas)

        scrollable_frame.bind(
            "<Configure>",
            lambda e: canvas.configure(
                scrollregion=canvas.bbox("all")
            )
        )

        canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)

        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        # Network settings
        net_frame = ttk.LabelFrame(scrollable_frame, text="Network Settings")
        net_frame.pack(fill="x", padx=10, pady=10)

        ttk.Label(net_frame, text="Port:").grid(row=0, column=0, padx=5, pady=5, sticky="w")
        ttk.Entry(net_frame, textvariable=self.port, width=10).grid(row=0, column=1, padx=5, pady=5, sticky="w")

        ttk.Label(net_frame, text="Host:").grid(row=0, column=2, padx=5, pady=5, sticky="w")
        ttk.Entry(net_frame, textvariable=self.host, width=15).grid(row=0, column=3, padx=5, pady=5, sticky="w")

        # Map settings
        map_frame = ttk.LabelFrame(scrollable_frame, text="Map Settings")
        map_frame.pack(fill="x", padx=10, pady=10)

        ttk.Label(map_frame, text="Width:").grid(row=0, column=0, padx=5, pady=5, sticky="w")
        ttk.Entry(map_frame, textvariable=self.map_width, width=10).grid(row=0, column=1, padx=5, pady=5, sticky="w")

        ttk.Label(map_frame, text="Height:").grid(row=0, column=2, padx=5, pady=5, sticky="w")
        ttk.Entry(map_frame, textvariable=self.map_height, width=10).grid(row=0, column=3, padx=5, pady=5, sticky="w")

        ttk.Label(map_frame, text="Client Limit:").grid(row=1, column=0, padx=5, pady=5, sticky="w")
        ttk.Entry(map_frame, textvariable=self.client_limit, width=10).grid(row=1, column=1, padx=5, pady=5, sticky="w")

        ttk.Label(map_frame, text="Frequency:").grid(row=1, column=2, padx=5, pady=5, sticky="w")
        ttk.Entry(map_frame, textvariable=self.freq, width=10).grid(row=1, column=3, padx=5, pady=5, sticky="w")

        # Game settings
        game_frame = ttk.LabelFrame(scrollable_frame, text="Game Settings")
        game_frame.pack(fill="x", padx=10, pady=10)

        ttk.Label(game_frame, text="Match Duration:").grid(row=0, column=0, padx=5, pady=5, sticky="w")
        ttk.Entry(game_frame, textvariable=self.match_duration, width=10).grid(row=0, column=1, padx=5, pady=5, sticky="w")

        ttk.Label(game_frame, text="Auto Start:").grid(row=0, column=2, padx=5, pady=5, sticky="w")
        ttk.Combobox(game_frame, textvariable=self.auto_start, values=["on", "off"], width=5).grid(row=0, column=3, padx=5, pady=5, sticky="w")

        ttk.Label(game_frame, text="Display Eggs:").grid(row=1, column=0, padx=5, pady=5, sticky="w")
        ttk.Combobox(game_frame, textvariable=self.display_eggs, values=["true", "false"], width=5).grid(row=1, column=1, padx=5, pady=5, sticky="w")

        # GUI settings
        gui_frame = ttk.LabelFrame(scrollable_frame, text="GUI Settings")
        gui_frame.pack(fill="x", padx=10, pady=10)

        ttk.Label(gui_frame, text="GUI Mode:").grid(row=0, column=0, padx=5, pady=5, sticky="w")
        ttk.Combobox(gui_frame, textvariable=self.gui_mode, values=["2d", "3d"], width=5).grid(row=0, column=1, padx=5, pady=5, sticky="w")

        # Team settings
        team_frame = ttk.LabelFrame(scrollable_frame, text="Teams")
        team_frame.pack(fill="x", padx=10, pady=10)

        # Team list frame
        teams_list_frame = ttk.Frame(team_frame)
        teams_list_frame.pack(fill="x", padx=5, pady=5)

        self.teams_listbox = tk.Listbox(teams_list_frame, height=5, width=30)
        self.teams_listbox.pack(side=tk.LEFT, fill="x", expand=True)

        teams_scrollbar = ttk.Scrollbar(teams_list_frame, orient="vertical", command=self.teams_listbox.yview)
        self.teams_listbox.configure(yscrollcommand=teams_scrollbar.set)
        teams_scrollbar.pack(side=tk.RIGHT, fill="y")

        # Populate teams
        for team in self.teams:
            self.teams_listbox.insert(tk.END, team)
            self.team_vars[team] = tk.BooleanVar(value=True)
            self.ai_count_vars[team] = tk.IntVar(value=1)

        # Team controls
        team_controls = ttk.Frame(team_frame)
        team_controls.pack(fill="x", padx=5, pady=5)

        self.new_team_entry = ttk.Entry(team_controls, width=20)
        self.new_team_entry.pack(side=tk.LEFT, padx=5)

        ttk.Button(team_controls, text="Add Team", command=self._add_team).pack(side=tk.LEFT, padx=5)
        ttk.Button(team_controls, text="Remove Selected", command=self._remove_team).pack(side=tk.LEFT, padx=5)

        # AI configuration for teams
        self.ai_config_frame = ttk.LabelFrame(scrollable_frame, text="AI Configuration")
        self.ai_config_frame.pack(fill="x", padx=10, pady=10)

        # Add checkboxes and AI count for each team
        self._update_ai_config_ui()

        # Terminal UI for AI
        ttk.Checkbutton(self.ai_config_frame, text="Enable Terminal UI for AI", variable=self.ai_terminal_ui).pack(anchor="w", padx=5, pady=5)

        # Control buttons
        control_frame = ttk.Frame(scrollable_frame)
        control_frame.pack(fill="x", padx=10, pady=10)

        ttk.Button(control_frame, text="Start Server", command=self.start_server).pack(side=tk.LEFT, padx=5)
        ttk.Button(control_frame, text="Start GUI", command=self.start_gui).pack(side=tk.LEFT, padx=5)
        ttk.Button(control_frame, text="Start AI", command=self.start_ai).pack(side=tk.LEFT, padx=5)
        ttk.Button(control_frame, text="Start All", command=self.start_all).pack(side=tk.LEFT, padx=5)
        ttk.Button(control_frame, text="Stop All", command=self.stop_all).pack(side=tk.LEFT, padx=5)

    def _update_ai_config_ui(self):
        # Clear previous widgets
        for widget in self.ai_config_frame.winfo_children():
            widget.destroy()

        # Create AI configuration for each team
        for i, team in enumerate(self.teams):
            team_frame = ttk.Frame(self.ai_config_frame)
            team_frame.pack(fill="x", padx=5, pady=2)

            ttk.Checkbutton(team_frame, text=team, variable=self.team_vars[team]).pack(side=tk.LEFT, padx=5)

            ttk.Label(team_frame, text="Number of AI:").pack(side=tk.LEFT, padx=5)
            ttk.Spinbox(team_frame, from_=0, to=10, width=3, textvariable=self.ai_count_vars[team]).pack(side=tk.LEFT, padx=5)

    def _add_team(self):
        team_name = self.new_team_entry.get().strip()
        if team_name and team_name not in self.teams:
            self.teams.append(team_name)
            self.teams_listbox.insert(tk.END, team_name)
            self.team_vars[team_name] = tk.BooleanVar(value=True)
            self.ai_count_vars[team_name] = tk.IntVar(value=1)
            self._update_ai_config_ui()
            self.new_team_entry.delete(0, tk.END)

    def _remove_team(self):
        selected = self.teams_listbox.curselection()
        if selected:
            index = selected[0]
            team_name = self.teams[index]
            del self.teams[index]
            del self.team_vars[team_name]
            del self.ai_count_vars[team_name]
            self.teams_listbox.delete(index)
            self._update_ai_config_ui()

    def _create_logs_tab(self):
        self.log_text = scrolledtext.ScrolledText(self.tab_logs, wrap=tk.WORD)
        self.log_text.pack(fill="both", expand=True, padx=10, pady=10)
        self.log_text.config(state=tk.DISABLED)

    def log(self, message):
        self.log_text.config(state=tk.NORMAL)
        self.log_text.insert(tk.END, message + "\n")
        self.log_text.see(tk.END)
        self.log_text.config(state=tk.DISABLED)

    def get_active_teams(self):
        return [team for team in self.teams if self.team_vars[team].get()]

    def start_server(self):
        if self.server_process:
            self.log("Server is already running")
            return

        # Get active teams
        active_teams = self.get_active_teams()
        if not active_teams:
            self.log("Error: At least one team must be enabled")
            return

        # Build the command
        server_path = os.path.join(os.getcwd(), "zappy_ref-v3.0.1", "linux", "zappy_server")
        cmd = [
            server_path,
            "-p", self.port.get(),
            "-x", self.map_width.get(),
            "-y", self.map_height.get(),
            "-n", *active_teams,
            "-c", self.client_limit.get(),
            "-f", self.freq.get(),
            "--auto-start", self.auto_start.get(),
            "--display-eggs", self.display_eggs.get(),
            "--match_duration", self.match_duration.get()
        ]

        cmd_str = " ".join(cmd)
        self.log(f"Starting server with command: {cmd_str}")

        try:
            self.server_process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True
            )
            self.log("Server started successfully")
        except Exception as e:
            self.log(f"Error starting server: {e}")

    def start_gui(self):
        if self.gui_process:
            self.log("GUI is already running")
            return

        # Build the command
        gui_path = os.path.join(os.getcwd(), "GUI", "zappy_gui")
        cmd = [
            gui_path,
            "-p", self.port.get(),
            "-h", self.host.get()
        ]

        # Add 2d/3d flag
        if self.gui_mode.get() == "2d":
            cmd.append("-2d")

        cmd_str = " ".join(cmd)
        self.log(f"Starting GUI with command: {cmd_str}")

        try:
            self.gui_process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True
            )
            self.log("GUI started successfully")
        except Exception as e:
            self.log(f"Error starting GUI: {e}")

    def start_ai(self):
        # Get active teams
        active_teams = self.get_active_teams()
        if not active_teams:
            self.log("Error: At least one team must be enabled")
            return

        # Build the command
        ai_path = os.path.join(os.getcwd(), "AI", "zappy_ai")

        for team in active_teams:
            ai_count = self.ai_count_vars[team].get()
            for _ in range(ai_count):
                cmd = [
                    ai_path,
                    "-p", self.port.get(),
                    "-h", self.host.get(),
                    "-n", team
                ]

                if self.ai_terminal_ui.get():
                    cmd.append("--terminal-ui")

                cmd_str = " ".join(cmd)
                self.log(f"Starting AI for team {team} with command: {cmd_str}")

                try:
                    ai_process = subprocess.Popen(
                        cmd,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.STDOUT,
                        text=True
                    )
                    self.ai_processes.append(ai_process)
                    self.log(f"AI for team {team} started successfully")
                except Exception as e:
                    self.log(f"Error starting AI for team {team}: {e}")

    def start_all(self):
        self.start_server()
        # Sleep briefly to allow the server to start
        self.root.after(1000, self.start_gui)
        # Sleep again before starting AI
        self.root.after(2000, self.start_ai)

    def stop_all(self):
        # Stop server
        if self.server_process:
            self.server_process.terminate()
            self.server_process = None
            self.log("Server stopped")

        # Stop GUI
        if self.gui_process:
            self.gui_process.terminate()
            self.gui_process = None
            self.log("GUI stopped")

        # Stop all AI processes
        for i, proc in enumerate(self.ai_processes):
            try:
                proc.terminate()
                self.log(f"AI process {i+1} stopped")
            except:
                pass
        self.ai_processes = []
        self.log("All processes stopped")

    def on_close(self):
        self.stop_all()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = ZappyLauncher(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()