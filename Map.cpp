#include "Map.hpp"
#include <cmath>

Map::Map(int mapWidth, int mapHeight, int tileSz)
    : width(mapWidth), height(mapHeight), tileSize(tileSz) {

    tiles.resize(height);
    for (int y = 0; y < height; y++) {
        tiles[y].resize(width);
        for (int x = 0; x < width; x++) {
            tiles[y][x].position = {(float)x, (float)y};
            tiles[y][x].baseColor = ((x + y) % 2 == 0) ? DARKGREEN : GREEN;
            tiles[y][x].hasResource = false;
            tiles[y][x].hasPlayer = false;
            tiles[y][x].resourceType = -1;
            tiles[y][x].playerCount = 0;
        }
    }
}

Map::~Map() {
    // Destructeur automatique pour les vectors
}

void Map::draw(Camera2D& camera) {
    // Calculer les tiles visibles pour optimiser le rendu
    Vector2 topLeft = GetScreenToWorld2D({0, 0}, camera);
    Vector2 bottomRight = GetScreenToWorld2D({(float)GetScreenWidth(), (float)GetScreenHeight()}, camera);

    int startX = std::max(0, (int)(topLeft.x / tileSize) - 1);
    int endX = std::min(width, (int)(bottomRight.x / tileSize) + 2);
    int startY = std::max(0, (int)(topLeft.y / tileSize) - 1);
    int endY = std::min(height, (int)(bottomRight.y / tileSize) + 2);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            Vector2 worldPos = getWorldPosition(x, y);
            Rectangle tileRect = {worldPos.x, worldPos.y, (float)tileSize, (float)tileSize};

            Color tileColor = tiles[y][x].baseColor;

            // Modifier la couleur si il y a des joueurs
            if (tiles[y][x].hasPlayer) {
                tileColor = ColorTint(tileColor, {255, 255, 255, 200});
            }

            DrawRectangleRec(tileRect, tileColor);
            DrawRectangleLinesEx(tileRect, 1.0f, DARKGRAY);

            // Dessiner le nombre de joueurs s'il y en a plusieurs
            if (tiles[y][x].playerCount > 1) {
                DrawText(TextFormat("%d", tiles[y][x].playerCount),
                        (int)worldPos.x + 2, (int)worldPos.y + 2, 12, BLACK);
            }
        }
    }
}

Vector2 Map::getWorldPosition(int x, int y) {
    return {(float)(x * tileSize), (float)(y * tileSize)};
}

Vector2 Map::getTileCoords(Vector2 worldPos) {
    return {(float)((int)worldPos.x / tileSize), (float)((int)worldPos.y / tileSize)};
}

Tile& Map::getTile(int x, int y) {
    // Gestion du monde sphérique (wrapping)
    x = ((x % width) + width) % width;
    y = ((y % height) + height) % height;
    return tiles[y][x];
}

void Map::setTileResource(int x, int y, int resourceType) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        tiles[y][x].hasResource = (resourceType >= 0);
        tiles[y][x].resourceType = resourceType;
    }
}

void Map::setTilePlayer(int x, int y, int playerCount) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        tiles[y][x].hasPlayer = (playerCount > 0);
        tiles[y][x].playerCount = playerCount;
    }
}

int Map::getWidth() const {
    return width;
}

int Map::getHeight() const {
    return height;
}

int Map::getTileSize() const {
    return tileSize;
}