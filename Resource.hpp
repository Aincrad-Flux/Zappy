#pragma once

#include <raylib.h>
#include <string>

enum class ResourceType {
    FOOD = 0,
    LINEMATE = 1,
    DERAUMERE = 2,
    SIBUR = 3,
    MENDIANE = 4,
    PHIRAS = 5,
    THYSTAME = 6
};

class Resource {
private:
    ResourceType type;
    Vector2 position;
    Color color;
    std::string name;

public:
    Resource(ResourceType resType, Vector2 pos);
    ~Resource();

    void draw(Vector2 worldPos, int tileSize) const;

    ResourceType getType() const;
    Vector2 getPosition() const;
    Color getColor() const;
    std::string getName() const;

    static Color getResourceColor(ResourceType type);
    static std::string getResourceName(ResourceType type);
};