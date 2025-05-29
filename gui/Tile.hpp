#pragma once
#include <raylib.h>

/**
 * @class Tile
 * @brief Represents a single tile on the game map
 */
class Tile {
private:
    Vector3 m_position;   ///< Position in 3D world space
    Color m_baseColor;    ///< Base color of the tile
    bool m_hasResource;   ///< Whether the tile has a resource
    bool m_hasPlayer;     ///< Whether a player is on the tile
    int m_resourceType;   ///< Type of resource on the tile
    int m_playerCount;    ///< Number of players on the tile

public:
    /**
     * @brief Constructor for Tile
     * @param position Position in 3D world space
     * @param baseColor Base color of the tile
     * @param hasResource Whether the tile has a resource
     * @param hasPlayer Whether a player is on the tile
     * @param resourceType Type of resource on the tile
     * @param playerCount Number of players on the tile
     */
    Tile(Vector3 position = {0, 0, 0},
         Color baseColor = WHITE,
         bool hasResource = false,
         bool hasPlayer = false,
         int resourceType = 0,
         int playerCount = 0);

    /**
     * @brief Destructor for Tile
     */
    ~Tile();

    // Getters
    Vector3 getPosition() const { return m_position; }
    Color getBaseColor() const { return m_baseColor; }
    bool getHasResource() const { return m_hasResource; }
    bool getHasPlayer() const { return m_hasPlayer; }
    int getResourceType() const { return m_resourceType; }
    int getPlayerCount() const { return m_playerCount; }

    // Setters
    void setPosition(const Vector3& position) { m_position = position; }
    void setBaseColor(const Color& color) { m_baseColor = color; }
    void setHasResource(bool hasResource) { m_hasResource = hasResource; }
    void setHasPlayer(bool hasPlayer) { m_hasPlayer = hasPlayer; }
    void setResourceType(int resourceType) { m_resourceType = resourceType; }
    void setPlayerCount(int count) { m_playerCount = count; }

    // Utility methods
    void incrementPlayerCount() { m_playerCount++; }
    void decrementPlayerCount() {
        if (m_playerCount > 0) m_playerCount--;
        m_hasPlayer = (m_playerCount > 0);
    }
    void addResource(int type) {
        m_resourceType = type;
        m_hasResource = true;
    }
    void removeResource() {
        m_resourceType = 0;
        m_hasResource = false;
    }
};
