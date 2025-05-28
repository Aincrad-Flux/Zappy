#pragma once

#include <raylib.h>
#include <vector>

struct Tile {
    Vector3 position;
    Color baseColor;
    bool hasResource;
    bool hasPlayer;
    int resourceType;
    int playerCount;
};

class Map {
private:
    int width;
    int height;
    int tileSize;
    std::vector<std::vector<Tile>> tiles;

public:
    Map(int mapWidth = 20, int mapHeight = 15, int tileSz = 32);
    ~Map();

    void draw();
    Vector3 getWorldPosition(int x, int y);
    Vector2 getTileCoords(Vector3 worldPos);
    Tile& getTile(int x, int y);
    void setTileResource(int x, int y, int resourceType);
    void setTilePlayer(int x, int y, int playerCount);

    int getWidth() const;
    int getHeight() const;
    int getTileSize() const;
};