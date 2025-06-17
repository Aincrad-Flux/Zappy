import sys
import time
from network import NetworkClient
from game_state import GameState
from decision import BasicAI
from utils import delay

def main():
    if len(sys.argv) < 3:
        print("Usage: python main.py <host> <port>")
        return

    port = int(sys.argv[1])
    host = sys.argv[2]

    print(f"[INFO] Connecting to {host}:{port}...")
    net = NetworkClient(host, port)
    if not net:
        print("[ERROR] Failed to create network client.")
        return
    net.connect()
    print("[INFO] Connected.")

    game_state = GameState()
    ai = BasicAI(game_state)

    authenticated = False
    team_name = sys.argv[3]
    print("[INFO] Waiting for WELCOME...")

    while not authenticated:
        for line in net.receive_response():
            print(f"[RECV] {line.strip()}")
            if "WELCOME" in line:
                print(f"[SEND] Sending team name: {team_name}")
                net.send_command(team_name)
            elif line.strip().isdigit():
                print("[INFO] Server accepted team, authenticated.")
                authenticated = True
        delay(50)

    print("[INFO] Entering main loop.")
    while True:
        for response in net.receive_response():
            print(f"[RECV] {response.strip()}")
            if response.startswith("["):
                game_state.update_inventory(response)

        action = ai.decide_next_action()
        print(f"[DECISION] Chosen action: {action}")
        net.send_command(action)

        if time.time() % 5 < 0.1:
            print("[AUTO] Sending 'Inventory'")
            net.send_command("Inventory")

        delay(200)

if __name__ == "__main__":
    main()
