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
   - **Level 1**: Performs elevation independently without broadcasting (requires only 1 player)
   - **Levels 2-7**: Broadcasts to coordinate with teammates when required resources are collected
4. **Reproduction**: Forks new players periodically to increase team presence

## Implementation Details

The AI uses a state machine to determine its current goal:
- SEARCHING_FOOD: When food levels are low
- COLLECTING_RESOURCES: When missing resources for the next level
- SEARCHING_RESOURCES: When exploring for resources
- ELEVATION: When attempting to level up

Team coordination is achieved through encrypted broadcast messages that share:
- Current level and elevation attempts
- Resource availability and needs
- Position information
- Level-up announcements

## Communication Protocol

The AI uses an encrypted communication system with the following features:
- Team-specific encryption to ensure only teammates can understand messages
- Message format: `TEAM:ID:TYPE:DATA`
- Message types:
  - ELEVATION: Request for teammates to join elevation ritual
  - ELEVATION_RESPONSE: Acknowledgment to elevation request
  - RESOURCES: Share inventory information
  - POSITION: Share location information
  - LEVEL_UP: Announce successful level up
  - NEED_RESOURCES: Request specific resources from teammates
  - RESOURCE_OFFER: Offer to share excess resources

Messages are encrypted using XOR and base64 encoding with a team-specific key to prevent
other teams from understanding or interfering with coordination.

## Level 2 and Higher Strategy

For levels 2 and higher, which require multiple players (from 2 to 6 depending on level), the AI implements this coordination strategy:

1. **Resource Collection Phase**:
   - AI collects all resources needed for the target level
   - It keeps track of what resources are still needed via periodic inventory checks

2. **Elevation Preparation Phase**:
   - Once all resources are collected, the AI enters the "ELEVATION" state
   - It broadcasts its position and readiness to teammates
   - It waits for responses from teammates who can join the ritual

3. **Coordination Phase**:
   - When other AIs respond, the initiating AI adds them to its list of participants
   - It waits for them to physically move to its tile
   - Once enough teammates are present on the same tile, it proceeds

4. **Ritual Phase**:
   - Places all required resources on the ground
   - Performs the incantation command
   - Upon successful elevation, broadcasts success to the team
   - Returns to resource collection for the next level

The number of players required for each level:
- Level 2: 2 players
- Level 3: 2 players
- Level 4: 4 players
- Level 5: 4 players
- Level 6: 6 players
- Level 7: 6 players

## Level 3 Strategy

Level 3 requires particularly effective team coordination with the following requirements:
- 2 players on the same tile
- 2 linemate, 0 deraumere, 1 sibur, 0 mendiane, 2 phiras, 0 thystame

The AI implements these specific strategies for Level 3:
1. More frequent broadcasts when ready for elevation
2. Detailed resource sharing to help teammates understand what's needed
3. Coordination messages specifically for Level 3 elevation
4. Persistently tracking teammates who have responded to elevation requests
5. Using position information to help teammates locate the elevation tile

When a player has all resources for Level 3, it:
1. Encrypts and broadcasts its readiness and resources
2. Waits for responses from teammates
3. Tracks which teammates are coming to help
4. Places resources only when enough teammates are present
5. Initiates the elevation ritual when all conditions are met
6. Broadcasts success to all teammates after elevation
