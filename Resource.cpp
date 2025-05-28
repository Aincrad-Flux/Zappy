#include "Resource.hpp"

Resource::Resource(ResourceType resType, Vector2 pos) : type(resType), position(pos) {
    color = getResourceColor(resType);
    name = getResourceName(resType);
}

Resource::~Resource() {
    // Rien à faire
}

void Resource::draw(Vector2 worldPos, int tileSize) const {
    Vector2 center = {worldPos.x + tileSize/2.0f, worldPos.y + tileSize/2.0f};

    // Différentes formes selon le type de ressource
    switch (type) {
        case ResourceType::FOOD:
            // Cercle pour la nourriture
            DrawCircleV(center, 6, color);
            DrawCircleLinesV(center, 6, BLACK);
            break;

        case ResourceType::LINEMATE:
            // Triangle pour linemate
            DrawTriangle({center.x, center.y - 8}, {center.x - 6, center.y + 4}, {center.x + 6, center.y + 4}, color);
            DrawTriangleLines({center.x, center.y - 8}, {center.x - 6, center.y + 4}, {center.x + 6, center.y + 4}, BLACK);
            break;

        case ResourceType::DERAUMERE:
            // Carré pour deraumere
            DrawRectangle((int)(center.x - 6), (int)(center.y - 6), 12, 12, color);
            DrawRectangleLines((int)(center.x - 6), (int)(center.y - 6), 12, 12, BLACK);
            break;

        case ResourceType::SIBUR:
            // Diamant pour sibur
            DrawPoly(center, 4, 8, 45, color);
            DrawPolyLines(center, 4, 8, 45, BLACK);
            break;

        case ResourceType::MENDIANE:
            // Hexagone pour mendiane
            DrawPoly(center, 6, 7, 0, color);
            DrawPolyLines(center, 6, 7, 0, BLACK);
            break;

        case ResourceType::PHIRAS:
            // Étoile pour phiras
            DrawPoly(center, 5, 8, 0, color);
            DrawPolyLines(center, 5, 8, 0, BLACK);
            break;

        case ResourceType::THYSTAME:
            // Croix pour thystame
            DrawRectangle((int)(center.x - 2), (int)(center.y - 8), 4, 16, color);
            DrawRectangle((int)(center.x - 8), (int)(center.y - 2), 16, 4, color);
            DrawRectangleLines((int)(center.x - 2), (int)(center.y - 8), 4, 16, BLACK);
            DrawRectangleLines((int)(center.x - 8), (int)(center.y - 2), 16, 4, BLACK);
            break;
    }
}

ResourceType Resource::getType() const {
    return type;
}

Vector2 Resource::getPosition() const {
    return position;
}

Color Resource::getColor() const {
    return color;
}

std::string Resource::getName() const {
    return name;
}

Color Resource::getResourceColor(ResourceType type) {
    switch (type) {
        case ResourceType::FOOD: return BROWN;
        case ResourceType::LINEMATE: return LIGHTGRAY;
        case ResourceType::DERAUMERE: return BLUE;
        case ResourceType::SIBUR: return YELLOW;
        case ResourceType::MENDIANE: return PURPLE;
        case ResourceType::PHIRAS: return ORANGE;
        case ResourceType::THYSTAME: return PINK;
        default: return WHITE;
    }
}

std::string Resource::getResourceName(ResourceType type) {
    switch (type) {
        case ResourceType::FOOD: return "food";
        case ResourceType::LINEMATE: return "linemate";
        case ResourceType::DERAUMERE: return "deraumere";
        case ResourceType::SIBUR: return "sibur";
        case ResourceType::MENDIANE: return "mendiane";
        case ResourceType::PHIRAS: return "phiras";
        case ResourceType::THYSTAME: return "thystame";
        default: return "unknown";
    }
}