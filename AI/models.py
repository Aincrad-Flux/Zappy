#!/usr/bin/env python3

"""
Model classes for the Zappy AI
"""

from enum import Enum

class ResourceType(Enum):
    """Resource types in the Zappy world"""
    FOOD = 0
    LINEMATE = 1
    DERAUMERE = 2
    SIBUR = 3
    MENDIANE = 4
    PHIRAS = 5
    THYSTAME = 6

class Direction(Enum):
    """Direction on the map"""
    NORTH = 1
    EAST = 2
    SOUTH = 3
    WEST = 4

# Elevation requirements per level
# Format: Level: (players_needed, linemate, deraumere, sibur, mendiane, phiras, thystame)
ELEVATION_REQUIREMENTS = {
    1: (1, 1, 0, 0, 0, 0, 0),
    2: (2, 1, 1, 1, 0, 0, 0),
    3: (2, 2, 0, 1, 0, 2, 0),
    4: (4, 1, 1, 2, 0, 1, 0),
    5: (4, 1, 2, 1, 3, 0, 0),
    6: (6, 1, 2, 3, 0, 1, 0),
    7: (6, 2, 2, 2, 2, 2, 1)
}

def get_resource_name(resource_type):
    """Convert ResourceType to string name used by the server"""
    if resource_type == ResourceType.FOOD:
        return "food"
    elif resource_type == ResourceType.LINEMATE:
        return "linemate"
    elif resource_type == ResourceType.DERAUMERE:
        return "deraumere"
    elif resource_type == ResourceType.SIBUR:
        return "sibur"
    elif resource_type == ResourceType.MENDIANE:
        return "mendiane"
    elif resource_type == ResourceType.PHIRAS:
        return "phiras"
    elif resource_type == ResourceType.THYSTAME:
        return "thystame"
    return ""
