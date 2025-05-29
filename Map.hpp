#pragma once

#include <raylib.h>
#include <vector>
#include "Tile.hpp"

/**
 * @class Map
 * @brief Represents the game world map
 *
 * This class handles the grid-based map and provides methods for
 * interacting with tiles and positioning entities.
 */
class Map {
private:
    int width;                            ///< Width of the map in tiles
    int height;                           ///< Height of the map in tiles
    int tileSize;                         ///< Size of each tile
    std::vector<std::vector<Tile>> tiles; ///< 2D grid of tiles

public:
    /**
     * @brief Constructor for Map
     * @param mapWidth Width of the map in tiles
     * @param mapHeight Height of the map in tiles
     * @param tileSz Size of each tile
     */
    Map(int mapWidth = 20, int mapHeight = 15, int tileSz = 32);

    /**
     * @brief Destructor for Map
     */
    ~Map();

    /**
     * @brief Draws the map on screen
     */
    void draw();

    /**
     * @brief Converts grid coordinates to world position
     * @param x X coordinate in the grid
     * @param y Y coordinate in the grid
     * @return World position as a Vector3
     */
    Vector3 getWorldPosition(int x, int y);

    /**
     * @brief Converts world position to grid coordinates
     * @param worldPos Position in world space
     * @return Grid coordinates as a Vector2
     */
    Vector2 getTileCoords(Vector3 worldPos);

    /**
     * @brief Gets a reference to a tile at specific coordinates
     * @param x X coordinate in the grid
     * @param y Y coordinate in the grid
     * @return Reference to the tile
     */
    Tile& getTile(int x, int y);

    /**
     * @brief Sets resource information for a tile
     * @param x X coordinate in the grid
     * @param y Y coordinate in the grid
     * @param resourceType Type of resource to set
     */
    void setTileResource(int x, int y, int resourceType);

    /**
     * @brief Updates player information for a tile
     * @param x X coordinate in the grid
     * @param y Y coordinate in the grid
     * @param playerCount Number of players on the tile
     */
    void setTilePlayer(int x, int y, int playerCount);

    int getWidth() const;
    int getHeight() const;
    int getTileSize() const;
};