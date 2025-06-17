import random

class BasicAI:
    def __init__(self, game_state):
        self.state = game_state
        self.commands = ["Forward", "Left", "Right"]

    def decide_next_action(self):
        if self.state.food < 5:
            return "Look"
        return random.choice(self.commands)
