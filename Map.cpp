#include "Map.hpp"
#include <cmath>
#include <algorithm>

Map::Map(int mapWidth, int mapHeight, int tileSz)
    : width(mapWidth), height(mapHeight), tileSize(tileSz)
{
    tiles.resize(height);
    for (int y = 0; y < height; y++) {
        tiles[y].resize(width);
        for (int x = 0; x < width; x++) {
            tiles[y][x].position = {(float)x, 0.0f, (float)y};
            tiles[y][x].baseColor = ((x + y) % 2 == 0) ? DARKGREEN : GREEN;
            tiles[y][x].hasResource = false;
            tiles[y][x].hasPlayer = false;
            tiles[y][x].resourceType = -1;
            tiles[y][x].playerCount = 0;
        }
    }
}

Map::~Map() {}

void Map::draw()
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Vector3 worldPos = getWorldPosition(x, y);
            Color tileColor = tiles[y][x].baseColor;

            if (tiles[y][x].hasPlayer) {
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

            if (tiles[y][x].playerCount > 1) {
                Vector3 indicatorPos = {worldPos.x + tileSize/2.0f, 0.5f, worldPos.z + tileSize/2.0f};
                DrawSphere(indicatorPos, tileSize * 0.15f, RED);

                for (int i = 1; i < std::min(tiles[y][x].playerCount, 4); i++) {
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
    return tiles[y][x];
}

void Map::setTileResource(int x, int y, int resourceType)
{
    if (x >= 0 && x < width && y >= 0 && y < height) {
        tiles[y][x].hasResource = (resourceType >= 0);
        tiles[y][x].resourceType = resourceType;
    }
}

void Map::setTilePlayer(int x, int y, int playerCount)
{
    if (x >= 0 && x < width && y >= 0 && y < height) {
        tiles[y][x].hasPlayer = (playerCount > 0);
        tiles[y][x].playerCount = playerCount;
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