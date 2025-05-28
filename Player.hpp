#pragma once

#include <raylib.h>
#include <string>
#include <vector>

enum class PlayerDirection {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3
};

struct Inventory {
    int food;
    int linemate;
    int deraumere;
    int sibur;
    int mendiane;
    int phiras;
    int thystame;
};

class Player {
private:
    int id;
    std::string teamName;
    Vector3 position;
    PlayerDirection direction;
    int level;
    Inventory inventory;
    Color teamColor;
    bool isAlive;
    float lifeTime;
    bool isIncanting;

public:
    Player(int playerId, const std::string& team, Vector3 pos, Color color);
    ~Player();

    void draw(Vector3 worldPos, int tileSize) const;
    void update(float deltaTime);
    void move(Vector3 newPos);
    void rotate(PlayerDirection newDir);
    void setLevel(int newLevel);
    void setIncanting(bool incanting);

    // Getters
    int getId() const;
    std::string getTeamName() const;
    Vector3 getPosition() const;
    PlayerDirection getDirection() const;
    int getLevel() const;
    Inventory getInventory() const;
    bool getIsAlive() const;
    float getLifeTime() const;
    Color getTeamColor() const;
    bool getIsIncanting() const;

    // Setters pour l'inventaire
    void setInventory(const Inventory& inv);
    void addToInventory(int resourceType, int amount);
};