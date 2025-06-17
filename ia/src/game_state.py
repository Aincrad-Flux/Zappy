import random

class GameState:
    def __init__(self):
        self.food = 10
        self.last_inventory = []

    def update_inventory(self, response):
        if "[" in response:
            items = response.strip("[]\n ").split(", ")
            self.last_inventory = [item.split() for item in items if item]
            self.food = next((int(q) for q, name in self.last_inventory if name == "food"), 0)
