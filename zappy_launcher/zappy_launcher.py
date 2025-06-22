#!/usr/bin/env python3
import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import subprocess
import os
import sys
import shlex
import threading

class ZappyLauncher:
    def __init__(self, root):
        self.root = root
        self.root.title("Zappy Launcher")
        self.root.geometry("600x900")

        # Create tabs
        self.notebook = ttk.Notebook(root)
        self.tab_config = ttk.Frame(self.notebook)
        self.tab_build = ttk.Frame(self.notebook)
        self.tab_logs = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_config, text="Configuration")
        self.notebook.add(self.tab_build, text="Build")
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

        # Button states
        self.server_start_btn = None
        self.server_stop_btn = None
        self.gui_start_btn = None
        self.gui_stop_btn = None
        self.ai_start_btn = None
        self.ai_stop_btn = None
        self.start_all_btn = None
        self.stop_all_btn = None

        # Add terminal command variable based on platform
        self.terminal_cmd = self._get_terminal_command()

        # UI creation
        self._create_config_tab()
        self._create_build_tab()
        self._create_logs_tab()

    def _get_terminal_command(self):
        """Get the appropriate terminal command based on the platform"""
        if sys.platform.startswith('linux'):
            # Check common Linux terminals
            terminals = [
                ('gnome-terminal', ['--', 'bash', '-c']),
                ('xterm', ['-e', 'bash', '-c']),
                ('konsole', ['--', 'bash', '-c']),
                ('terminator', ['-e', 'bash -c']),
                ('xfce4-terminal', ['--', 'bash', '-c'])
            ]

            for term, args in terminals:
                try:
                    if subprocess.run(['which', term], stdout=subprocess.PIPE, stderr=subprocess.PIPE).returncode == 0:
                        return [term] + args
                except:
                    pass

            # Fallback to xterm which is usually available
            return ['xterm', '-e', 'bash', '-c']
        elif sys.platform == 'darwin':  # macOS
            return ['osascript', '-e', 'tell app "Terminal" to do script']
        else:  # Windows or other
            self.log("Warning: Separate terminal windows not supported on this platform")
            return None

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

        # Server controls
        server_frame = ttk.LabelFrame(control_frame, text="Server")
        server_frame.pack(side=tk.LEFT, padx=5, fill="y")
        self.server_start_btn = ttk.Button(server_frame, text="Start", command=self.start_server, width=10)
        self.server_start_btn.pack(side=tk.TOP, padx=5, pady=2)
        self.server_stop_btn = ttk.Button(server_frame, text="Stop", command=self.stop_server, width=10, state=tk.DISABLED)
        self.server_stop_btn.pack(side=tk.TOP, padx=5, pady=2)

        # GUI controls
        gui_frame = ttk.LabelFrame(control_frame, text="GUI")
        gui_frame.pack(side=tk.LEFT, padx=5, fill="y")
        self.gui_start_btn = ttk.Button(gui_frame, text="Start", command=self.start_gui, width=10)
        self.gui_start_btn.pack(side=tk.TOP, padx=5, pady=2)
        self.gui_stop_btn = ttk.Button(gui_frame, text="Stop", command=self.stop_gui, width=10, state=tk.DISABLED)
        self.gui_stop_btn.pack(side=tk.TOP, padx=5, pady=2)

        # AI controls
        ai_frame = ttk.LabelFrame(control_frame, text="AI")
        ai_frame.pack(side=tk.LEFT, padx=5, fill="y")
        self.ai_start_btn = ttk.Button(ai_frame, text="Start", command=self.start_ai, width=10)
        self.ai_start_btn.pack(side=tk.TOP, padx=5, pady=2)
        self.ai_stop_btn = ttk.Button(ai_frame, text="Stop", command=self.stop_ai, width=10, state=tk.DISABLED)
        self.ai_stop_btn.pack(side=tk.TOP, padx=5, pady=2)

        # Global controls
        global_frame = ttk.LabelFrame(control_frame, text="All Components")
        global_frame.pack(side=tk.LEFT, padx=5, fill="y")
        self.start_all_btn = ttk.Button(global_frame, text="Start All", command=self.start_all, width=10)
        self.start_all_btn.pack(side=tk.TOP, padx=5, pady=2)
        self.stop_all_btn = ttk.Button(global_frame, text="Stop All", command=self.stop_all, width=10, state=tk.DISABLED)
        self.stop_all_btn.pack(side=tk.TOP, padx=5, pady=2)

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

    def _create_build_tab(self):
        build_frame = ttk.Frame(self.tab_build)
        build_frame.pack(fill="both", expand=True, padx=10, pady=10)

        # Title and description
        ttk.Label(build_frame, text="Build Controls", font=("Arial", 12, "bold")).pack(pady=(0, 10))
        ttk.Label(build_frame, text="Execute Makefile rules to build and manage the project components").pack(pady=(0, 20))

        # Component build frame
        comp_frame = ttk.LabelFrame(build_frame, text="Component Build")
        comp_frame.pack(fill="x", padx=5, pady=10)

        # Create buttons for component builds with better descriptions
        ttk.Button(comp_frame, text="Build GUI",
                  command=lambda: self.run_make_command("zappy_gui"),
                  width=25).pack(side=tk.LEFT, padx=20, pady=10, fill="x", expand=True)

        ttk.Button(comp_frame, text="Build AI",
                  command=lambda: self.run_make_command("zappy_ai"),
                  width=25).pack(side=tk.LEFT, padx=20, pady=10, fill="x", expand=True)

        # Standard Makefile rules buttons
        rules_frame = ttk.LabelFrame(build_frame, text="Standard Build Rules")
        rules_frame.pack(fill="x", padx=5, pady=10)

        button_frame1 = ttk.Frame(rules_frame)
        button_frame1.pack(fill="x", padx=5, pady=5)

        ttk.Button(button_frame1, text="Build All",
                  command=lambda: self.run_make_command("all"),
                  width=20).pack(side=tk.LEFT, padx=10, pady=5, fill="x", expand=True)

        ttk.Button(button_frame1, text="Clean",
                  command=lambda: self.run_make_command("clean"),
                  width=20).pack(side=tk.LEFT, padx=10, pady=5, fill="x", expand=True)

        ttk.Button(button_frame1, text="Full Clean (fclean)",
                  command=lambda: self.run_make_command("fclean"),
                  width=20).pack(side=tk.LEFT, padx=10, pady=5, fill="x", expand=True)

        button_frame2 = ttk.Frame(rules_frame)
        button_frame2.pack(fill="x", padx=5, pady=5)

        ttk.Button(button_frame2, text="Rebuild (re)",
                  command=lambda: self.run_make_command("re"),
                  width=20).pack(side=tk.LEFT, padx=10, pady=5, fill="x", expand=True)

        ttk.Button(button_frame2, text="Debug Build",
                  command=lambda: self.run_make_command("debug"),
                  width=20).pack(side=tk.LEFT, padx=10, pady=5, fill="x", expand=True)

        ttk.Button(button_frame2, text="Show Build Help",
                  command=self.show_build_help,
                  width=20).pack(side=tk.LEFT, padx=10, pady=5, fill="x", expand=True)

        # Custom command entry
        custom_frame = ttk.LabelFrame(build_frame, text="Custom Make Command")
        custom_frame.pack(fill="x", padx=5, pady=10)

        custom_cmd_frame = ttk.Frame(custom_frame)
        custom_cmd_frame.pack(fill="x", padx=5, pady=5)

        ttk.Label(custom_cmd_frame, text="make ").pack(side=tk.LEFT)
        self.custom_make_command = ttk.Entry(custom_cmd_frame, width=30)
        self.custom_make_command.pack(side=tk.LEFT, padx=5, fill="x", expand=True)

        ttk.Button(custom_cmd_frame, text="Execute",
                  command=self.run_custom_make_command).pack(side=tk.LEFT, padx=5)

        # Progress status
        self.build_status = tk.StringVar(value="Ready")
        status_frame = ttk.Frame(build_frame)
        status_frame.pack(fill="x", padx=5, pady=5)
        ttk.Label(status_frame, text="Status: ").pack(side=tk.LEFT)
        ttk.Label(status_frame, textvariable=self.build_status).pack(side=tk.LEFT)

    def show_build_help(self):
        """Show help about available build options"""
        help_text = """
        Build Options:

        - Build GUI: Compiles only the GUI component (zappy_gui)

        - Build AI: Compiles only the AI component (zappy_ai)

        - Build All: Compiles all project components (GUI and AI)

        - Clean: Removes all object files but keeps binaries

        - Full Clean (fclean): Removes all generated files including binaries

        - Rebuild (re): Full clean followed by full build

        - Debug Build: Builds with debug flags

        You can also enter custom make commands in the field at the bottom.
        """
        messagebox.showinfo("Build Help", help_text)

    def run_make_command(self, rule):
        """Run a make command with the specified rule"""
        # Get the project root directory
        project_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

        command = ["make", "-C", project_dir, rule]

        self.log(f"Running 'make {rule}'...")
        self.build_status.set(f"Building: {rule}...")

        # Run the command in a separate thread to avoid blocking the UI
        threading.Thread(target=self._run_command_thread, args=(command, rule)).start()

    def run_custom_make_command(self):
        """Run a custom make command"""
        custom_rule = self.custom_make_command.get().strip()
        if not custom_rule:
            self.log("Error: Please enter a make command")
            return

        # Get the project root directory
        project_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

        command = ["make", "-C", project_dir] + shlex.split(custom_rule)

        self.log(f"Running 'make {custom_rule}'...")

        # Run the command in a separate thread to avoid blocking the UI
        threading.Thread(target=self._run_command_thread, args=(command,)).start()

    def _run_command_thread(self, command, rule=None):
        """Run a command in a thread and update the logs"""
        try:
            process = subprocess.Popen(
                command,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
                universal_newlines=True
            )

            # Read and display output line by line as it comes
            if process.stdout:
                for line in process.stdout:
                    self.log(line.strip())

            process.wait()

            if process.returncode == 0:
                self.log(f"Command completed successfully (return code: {process.returncode})")
                if rule:
                    self.build_status.set(f"{rule} completed successfully")
                else:
                    self.build_status.set("Command completed")
            else:
                self.log(f"Command failed with return code: {process.returncode}")
                self.build_status.set(f"Build failed (code: {process.returncode})")
        except Exception as e:
            self.log(f"Error executing command: {e}")
            self.build_status.set("Error executing command")

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

            # Update button states
            self.server_start_btn.configure(state=tk.DISABLED)
            self.server_stop_btn.configure(state=tk.NORMAL)
            self._update_global_button_states()
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

            # Update button states
            self.gui_start_btn.configure(state=tk.DISABLED)
            self.gui_stop_btn.configure(state=tk.NORMAL)
            self._update_global_button_states()
        except Exception as e:
            self.log(f"Error starting GUI: {e}")

    def start_ai(self):
        """Start AI clients in separate terminal windows or in background"""
        # Get active teams
        active_teams = self.get_active_teams()
        if not active_teams:
            self.log("Error: At least one team must be enabled")
            return

        # Get path to AI binary
        ai_path = os.path.join(os.getcwd(), "AI", "zappy_ai")

        # Vérifier que le fichier existe
        if not os.path.isfile(ai_path):
            ai_path = os.path.join(os.getcwd(), "zappy_ai")
            if not os.path.isfile(ai_path):
                self.log(f"Error: AI binary not found at {ai_path}")
                return

        # Make sure the AI binary is executable
        try:
            os.chmod(ai_path, os.stat(ai_path).st_mode | 0o111)
        except Exception as e:
            self.log(f"Warning: Could not set executable permission on AI binary: {e}")

        # Counter for AI instances
        ai_count = 0

        # Option to open in separate terminals
        open_in_terminal = True  # Added this option, could be made configurable

        for team in active_teams:
            instances = self.ai_count_vars[team].get()
            for i in range(instances):
                # Build the command
                ai_args = [
                    ai_path,
                    "-p", self.port.get(),
                    "-h", self.host.get(),
                    "-n", team
                ]

                if self.ai_terminal_ui.get():
                    ai_args.append("--terminal-ui")

                cmd_str = " ".join(ai_args)
                self.log(f"Starting AI #{ai_count+1} for team {team} with command: {cmd_str}")

                try:
                    if open_in_terminal and self.terminal_cmd:
                        # For terminal execution, we need to handle differently
                        if sys.platform.startswith('linux'):
                            # Get current directory for correct path resolution
                            cwd = os.getcwd()

                            # DEBUG: Print terminal detection info
                            self.log(f"Using terminal command: {self.terminal_cmd}")

                            # Form the command to run in terminal - CORRECTED APPROACH
                            command_inside_terminal = f"{ai_path} -p {self.port.get()} -h {self.host.get()} -n {team}"
                            if self.ai_terminal_ui.get():
                                command_inside_terminal += " --terminal-ui"

                            # More reliable way to execute commands in different terminal emulators
                            if self.terminal_cmd[0] == "gnome-terminal":
                                # For gnome-terminal, needs special handling
                                terminal_full_cmd = [
                                    "gnome-terminal",
                                    "--working-directory=" + cwd,
                                    "--", "bash", "-c",
                                    f"{command_inside_terminal}; echo 'AI process terminated. Press Enter to close.'; read"
                                ]
                            elif self.terminal_cmd[0] == "konsole":
                                # For KDE Konsole
                                terminal_full_cmd = [
                                    "konsole",
                                    "--workdir", cwd,
                                    "-e", "bash", "-c",
                                    f"{command_inside_terminal}; echo 'AI process terminated. Press Enter to close.'; read"
                                ]
                            else:
                                # For xterm, terminator, xfce4-terminal and others
                                terminal_full_cmd = [
                                    self.terminal_cmd[0],
                                    "-e", f"cd {cwd} && {command_inside_terminal}; echo 'AI process terminated. Press Enter to close.'; read"
                                ]

                            # Log the full command for debugging
                            self.log(f"Full terminal command: {terminal_full_cmd}")

                            # Start detached process without capturing output
                            subprocess.Popen(
                                terminal_full_cmd,
                                stdout=subprocess.PIPE,  # Changed from DEVNULL to capture potential errors
                                stderr=subprocess.PIPE,
                                start_new_session=True
                            )

                        elif sys.platform == 'darwin':  # macOS
                            # For macOS, it's a bit different with osascript
                            command_inside_terminal = f"cd {os.getcwd()} && {cmd_str}"
                            terminal_full_cmd = [
                                "osascript",
                                "-e",
                                f'tell app "Terminal" to do script "{command_inside_terminal}; echo \'AI process terminated. Press Enter to close.\'; read"'
                            ]
                            subprocess.Popen(terminal_full_cmd)

                        self.log(f"AI #{ai_count+1} for team {team} started in a new terminal window")
                    else:
                        # Original method - run in background
                        ai_process = subprocess.Popen(
                            ai_args,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT,
                            text=True
                        )
                        self.ai_processes.append(ai_process)
                        self.log(f"AI #{ai_count+1} for team {team} started in background")

                    ai_count += 1
                except Exception as e:
                    self.log(f"Error starting AI for team {team}: {e}")
                    import traceback
                    self.log(traceback.format_exc())

        self.log(f"Started {ai_count} AI clients")

        # Update button states
        if ai_count > 0:
            self.ai_start_btn.configure(state=tk.DISABLED)
            self.ai_stop_btn.configure(state=tk.NORMAL)
            self._update_global_button_states()

    def stop_all(self):
        """Stop all running processes: server, GUI, and AI"""
        # Stop server
        if self.server_process:
            self.server_process.terminate()
            self.server_process = None
            self.log("Server stopped")
            self.server_start_btn.configure(state=tk.NORMAL)
            self.server_stop_btn.configure(state=tk.DISABLED)

        # Stop GUI
        if self.gui_process:
            self.gui_process.terminate()
            self.gui_process = None
            self.log("GUI stopped")
            self.gui_start_btn.configure(state=tk.NORMAL)
            self.gui_stop_btn.configure(state=tk.DISABLED)

        # Stop all AI processes
        if self.ai_processes:
            for i, proc in enumerate(self.ai_processes):
                try:
                    proc.terminate()
                    self.log(f"AI process {i+1} stopped")
                except Exception as e:
                    self.log(f"Error stopping AI process {i+1}: {e}")
            self.ai_processes = []
            self.log("All AI processes stopped")
            self.ai_start_btn.configure(state=tk.NORMAL)
            self.ai_stop_btn.configure(state=tk.DISABLED)

        self.log("All processes stopped")

        # Update global button states
        self.start_all_btn.configure(state=tk.NORMAL)
        self.stop_all_btn.configure(state=tk.DISABLED)

    def start_all(self):
        """Start all components in sequence: server, GUI, then AI."""
        self.log("Starting all components...")

        # Disable the Start All button while starting components
        self.start_all_btn.configure(state=tk.DISABLED)
        self.start_server()

        # Sleep briefly to allow the server to start
        self.root.after(1000, self._start_gui_after_server)

    def _start_gui_after_server(self):
        """Helper method to start GUI after server has started"""
        self.start_gui()
        # Sleep again before starting AI
        self.root.after(2000, self._start_ai_after_gui)

    def _start_ai_after_gui(self):
        """Helper method to start AI after GUI has started"""
        self.start_ai()
        self.log("All components started")

    def on_close(self):
        self.stop_all()
        self.root.destroy()

    def stop_server(self):
        """Stop the server process if it's running"""
        if self.server_process:
            self.server_process.terminate()
            self.server_process = None
            self.log("Server stopped")

            # Update button states
            self.server_start_btn.configure(state=tk.NORMAL)
            self.server_stop_btn.configure(state=tk.DISABLED)
            self._update_global_button_states()
        else:
            self.log("Server is not running")

    def stop_gui(self):
        """Stop the GUI process if it's running"""
        if self.gui_process:
            self.gui_process.terminate()
            self.gui_process = None
            self.log("GUI stopped")

            # Update button states
            self.gui_start_btn.configure(state=tk.NORMAL)
            self.gui_stop_btn.configure(state=tk.DISABLED)
            self._update_global_button_states()
        else:
            self.log("GUI is not running")

    def stop_ai(self):
        """Stop all AI processes"""
        if not self.ai_processes:
            self.log("No AI processes are running")
            return

        for i, proc in enumerate(self.ai_processes):
            try:
                proc.terminate()
                self.log(f"AI process {i+1} stopped")
            except Exception as e:
                self.log(f"Error stopping AI process {i+1}: {e}")

        self.ai_processes = []
        self.log("All AI processes stopped")

        # Update button states
        self.ai_start_btn.configure(state=tk.NORMAL)
        self.ai_stop_btn.configure(state=tk.DISABLED)
        self._update_global_button_states()

    def _update_global_button_states(self):
        """Update global control buttons based on individual component states"""
        # Enable Start All if no components are running
        if not self.server_process and not self.gui_process and not self.ai_processes:
            self.start_all_btn.configure(state=tk.NORMAL)
            self.stop_all_btn.configure(state=tk.DISABLED)
        # Enable Stop All if any component is running
        elif self.server_process or self.gui_process or self.ai_processes:
            self.stop_all_btn.configure(state=tk.NORMAL)
            # Only disable Start All if all components are running
            all_running = (self.server_process is not None and
                          self.gui_process is not None and
                          len(self.ai_processes) > 0)
            if all_running:
                self.start_all_btn.configure(state=tk.DISABLED)

if __name__ == "__main__":
    root = tk.Tk()
    app = ZappyLauncher(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()