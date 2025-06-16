/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** Map.cpp
*/

#include "Map.hpp"
#include "Logger.hpp"
#include <cmath>
#include <algorithm>

Map::Map(int mapWidth, int mapHeight, int tileSz)
    : width(mapWidth), height(mapHeight), tileSize(tileSz)
{
    Logger::getInstance().info("Creating map with dimensions " + std::to_string(width) + "x" + std::to_string(height) + ", tile size: " + std::to_string(tileSize));
    
    tiles.resize(height);
    for (int y = 0; y < height; y++) {
        tiles[y].resize(width);
        for (int x = 0; x < width; x++) {
            tiles[y][x] = Tile(
                {(float)x, 0.0f, (float)y},          // position
                ((x + y) % 2 == 0) ? DARKGREEN : GREEN,  // baseColor
                false,                                // hasResource
                false,                                // hasPlayer
                -1,                                   // resourceType
                0                                     // playerCount
            );
        }
    }
    
    Logger::getInstance().info("Map created successfully");
}

Map::~Map() {
    Logger::getInstance().info("Map destroyed");
}

void Map::draw()
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Vector3 worldPos = getWorldPosition(x, y);
            Color tileColor = tiles[y][x].getBaseColor();

            if (tiles[y][x].getHasPlayer()) {
                tileColor.r = (tileColor.r + 255) / 2;
                tileColor.g = (tileColor.g + 255) / 2;
                tileColor.b = (tileColor.b + 255) / 2;
            }

            DrawCube(
                Vector3{worldPos.x + tileSize/2.0f, -0.1f, worldPos.z + tileSize/2.0f}, // centre
                (float)tileSize, 0.2f, (float)tileSize, // largeur, hauteur, profondeur
                tileColor); // couleur
            DrawLine3D(
                Vector3{worldPos.x, 0, worldPos.z},
                Vector3{worldPos.x + tileSize, 0, worldPos.z},
                DARKGRAY);
            DrawLine3D(
                Vector3{worldPos.x, 0, worldPos.z},
                Vector3{worldPos.x, 0, worldPos.z + tileSize},
                DARKGRAY);
            DrawLine3D(
                Vector3{worldPos.x + tileSize, 0, worldPos.z},
                Vector3{worldPos.x + tileSize, 0, worldPos.z + tileSize},
                DARKGRAY);
            DrawLine3D(
                Vector3{worldPos.x, 0, worldPos.z + tileSize},
                Vector3{worldPos.x + tileSize, 0, worldPos.z + tileSize},
                DARKGRAY);

            if (tiles[y][x].getPlayerCount() > 1) {
                Vector3 indicatorPos = {worldPos.x + tileSize/2.0f, 0.5f, worldPos.z + tileSize/2.0f};
                DrawSphere(indicatorPos, tileSize * 0.15f, RED);

                for (int i = 1; i < std::min(tiles[y][x].getPlayerCount(), 4); i++) {
                    Vector3 dotPos = {
                        indicatorPos.x + cosf((float)i * 3.14159f * 0.5f) * 0.2f * tileSize,
                        indicatorPos.y,
                        indicatorPos.z + sinf((float)i * 3.14159f * 0.5f) * 0.2f * tileSize
                    };
                    DrawSphere(dotPos, tileSize * 0.1f, WHITE);
                }
            }
        }
    }
}

Vector3 Map::getWorldPosition(int x, int y)
{
    return {(float)(x * tileSize), 0.0f, (float)(y * tileSize)};
}

Vector2 Map::getTileCoords(Vector3 worldPos)
{
    return {(float)((int)worldPos.x / tileSize), (float)((int)worldPos.z / tileSize)};
}

Tile& Map::getTile(int x, int y)
{
    x = ((x % width) + width) % width;
    y = ((y % height) + height) % height;
    
    // Only log if coordinates were wrapped
    if (x != x % width || y != y % height) {
        Logger::getInstance().debug("Tile coordinates wrapped: original (" + 
            std::to_string(x) + "," + std::to_string(y) + "), wrapped to (" + 
            std::to_string(x % width) + "," + std::to_string(y % height) + ")");
    }
    
    return tiles[y][x];
}

void Map::setTileResource(int x, int y, int resourceType, int count)
{
    if (x >= 0 && x < width && y >= 0 && y < height) {
        tiles[y][x].setHasResource(resourceType >= 0);
        tiles[y][x].setResourceType(resourceType);
        
        Logger::getInstance().debug("Tile resource set at (" + std::to_string(x) + "," + 
            std::to_string(y) + "), resource type: " + std::to_string(resourceType) + 
            ", count: " + std::to_string(count));
    } else {
        Logger::getInstance().warning("Attempted to set resource outside map bounds: (" + 
            std::to_string(x) + "," + std::to_string(y) + ")");
    }
}

void Map::setTilePlayer(int x, int y, int playerCount)
{
    if (x >= 0 && x < width && y >= 0 && y < height) {
        int oldCount = tiles[y][x].getPlayerCount();
        tiles[y][x].setHasPlayer(playerCount > 0);
        tiles[y][x].setPlayerCount(playerCount);
        
        if (oldCount != playerCount) {
            Logger::getInstance().debug("Tile player count updated at (" + std::to_string(x) + "," + 
                std::to_string(y) + "), from " + std::to_string(oldCount) + 
                " to " + std::to_string(playerCount));
        }
    } else {
        Logger::getInstance().warning("Attempted to set player count outside map bounds: (" + 
            std::to_string(x) + "," + std::to_string(y) + ")");
    }
}

int Map::getWidth() const
{
    return width;
}

int Map::getHeight() const
{
    return height;
}

int Map::getTileSize() const
{
    return tileSize;
}