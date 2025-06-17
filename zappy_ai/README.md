# Zappy AI Client

This is an AI client for the Zappy game. The AI's goal is to survive, collect resources, and level up through the elevation ritual.

## Features

- Automatic food search when hungry
- Resource collection based on next level requirements
- Elevation ritual coordination
- Team communication through broadcasts
- Automatic player replication through forking

## Requirements

- Python 3.6 or higher

## Usage

```bash
python -m zappy_ai.main -p <port> -n <team_name> [-h <hostname>]
```

### Arguments

- `-p, --port`: Port number (required)
- `-n, --name`: Name of the team (required)
- `-h, --host`: Name or IP of the machine; localhost by default

## AI Strategy

The AI follows these basic strategies:

1. **Survival**: Always prioritizes food when hunger levels are critical
2. **Resource Collection**: Searches for and collects resources needed for elevation
3. **Elevation**: Initiates the elevation ritual when all requirements are met
4. **Reproduction**: Forks new players periodically to increase team presence

## Implementation Details

The AI uses a state machine to determine its current goal:
- SEARCHING_FOOD: When food levels are low
- COLLECTING_RESOURCES: When missing resources for the next level
- SEARCHING_RESOURCES: When exploring for resources
- ELEVATION: When attempting to level up

Team coordination is achieved through broadcast messages that share:
- Current level and elevation attempts
- Resource availability
