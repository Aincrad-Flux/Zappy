#pragma once

#include <raylib.h>
#include <string>
#include <vector>

/**
 * @enum PlayerDirection
 * @brief Represents the direction a player is facing
 */
enum class PlayerDirection {
    NORTH = 0,  ///< Facing north
    EAST = 1,   ///< Facing east
    SOUTH = 2,  ///< Facing south
    WEST = 3    ///< Facing west
};

/**
 * @struct Inventory
 * @brief Contains the count of resources a player has
 */
struct Inventory {
    int food;       ///< Amount of food
    int linemate;   ///< Amount of linemate
    int deraumere;  ///< Amount of deraumere
    int sibur;      ///< Amount of sibur
    int mendiane;   ///< Amount of mendiane
    int phiras;     ///< Amount of phiras
    int thystame;   ///< Amount of thystame
};

/**
 * @class Player
 * @brief Represents a player in the Zappy game
 * 
 * This class handles player properties, rendering, and state management.
 */
class Player {
private:
    int id;                     ///< Unique player identifier
    std::string teamName;       ///< Name of the player's team
    Vector3 position;           ///< Position in 3D space
    PlayerDirection direction;  ///< Direction the player is facing
    int level;                  ///< Current level of the player
    Inventory inventory;        ///< Player's inventory of resources
    Color teamColor;            ///< Color representing the player's team
    bool isAlive;               ///< Whether the player is alive
    float lifeTime;             ///< Remaining lifetime of the player
    bool isIncanting;           ///< Whether the player is performing an incantation

public:
    /**
     * @brief Constructor for Player
     * @param playerId Unique identifier for the player
     * @param team Team name
     * @param pos Initial position
     * @param color Team color
     */
    Player(int playerId, const std::string& team, Vector3 pos, Color color);
    
    /**
     * @brief Destructor for Player
     */
    ~Player();

    /**
     * @brief Draws the player in the world
     * @param worldPos Base position in the world
     * @param tileSize Size of a map tile
     */
    void draw(Vector3 worldPos, int tileSize) const;
    
    /**
     * @brief Updates the player state
     * @param deltaTime Time since the last update
     */
    void update(float deltaTime);
    
    /**
     * @brief Moves the player to a new position
     * @param newPos The new position
     */
    void move(Vector3 newPos);
    
    /**
     * @brief Changes the player's direction
     * @param newDir The new direction
     */
    void rotate(PlayerDirection newDir);
    
    /**
     * @brief Sets the player's level
     * @param newLevel The new level
     */
    void setLevel(int newLevel);
    
    /**
     * @brief Sets whether the player is incanting
     * @param incanting True if incanting, false otherwise
     */
    void setIncanting(bool incanting);

    /**
     * @brief Gets the player's ID
     * @return The player's ID
     */
    int getId() const;
    
    /**
     * @brief Gets the player's team name
     * @return The player's team name
     */
    std::string getTeamName() const;
    
    /**
     * @brief Gets the player's position
     * @return The player's position
     */
    Vector3 getPosition() const;
    
    /**
     * @brief Gets the player's facing direction
     * @return The player's direction
     */
    PlayerDirection getDirection() const;
    
    /**
     * @brief Gets the player's level
     * @return The player's level
     */
    int getLevel() const;
    
    /**
     * @brief Gets the player's inventory
     * @return The player's inventory
     */
    Inventory getInventory() const;
    
    /**
     * @brief Checks if the player is alive
     * @return True if the player is alive, false otherwise
     */
    bool getIsAlive() const;
    
    /**
     * @brief Gets the player's remaining lifetime
     * @return The player's remaining lifetime
     */
    float getLifeTime() const;
    
    /**
     * @brief Gets the player's team color
     * @return The player's team color
     */
    Color getTeamColor() const;
    
    /**
     * @brief Checks if the player is currently incanting
     * @return True if incanting, false otherwise
     */
    bool getIsIncanting() const;

    /**
     * @brief Sets the player's inventory
     * @param inv The new inventory
     */
    void setInventory(const Inventory& inv);
    
    /**
     * @brief Adds a resource to the player's inventory
     * @param resourceType Type of resource (index)
     * @param amount Amount to add
     */
    void addToInventory(int resourceType, int amount);
};