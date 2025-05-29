#include "Resource.hpp"

Resource::Resource(ResourceType resType, Vector3 pos) : type(resType), position(pos)
{
    color = getResourceColor(resType);
    name = getResourceName(resType);
}

Resource::~Resource() {}

void Resource::draw(Vector3 worldPos, int tileSize) const
{
    Vector3 center = {worldPos.x + tileSize/2.0f, 0.3f, worldPos.z + tileSize/2.0f};
    const float scaleFactor = 10.0f;

    switch (type)
    {
        case ResourceType::FOOD:
            DrawSphere(center, 0.6f * scaleFactor, color);
            DrawSphereWires(center, 0.6f * scaleFactor, 8, 8, BLACK);
            break;

        case ResourceType::LINEMATE:
            DrawCube(center, 0.8f * scaleFactor, 0.8f * scaleFactor, 0.8f * scaleFactor, color);
            DrawCubeWires(center, 0.8f * scaleFactor, 0.8f * scaleFactor, 0.8f * scaleFactor, BLACK);
            break;

        case ResourceType::DERAUMERE: {
            DrawCube(center, 0.6f * scaleFactor, 0.4f * scaleFactor, 0.6f * scaleFactor, color);
            DrawCubeWires(center, 0.6f * scaleFactor, 0.4f * scaleFactor, 0.6f * scaleFactor, BLACK);

            Vector3 topCenter = {center.x, center.y + 0.4f * scaleFactor, center.z};
            DrawCube(topCenter, 0.3f * scaleFactor, 0.3f * scaleFactor, 0.3f * scaleFactor, color);
            DrawCubeWires(topCenter, 0.3f * scaleFactor, 0.3f * scaleFactor, 0.3f * scaleFactor, BLACK);
            break;
        }
        case ResourceType::SIBUR: {
            float size = 0.7f * scaleFactor;

            DrawCube(center, size, size, size, color);
            DrawCubeWires(center, size, size, size, BLACK);
            DrawSphere(center, 0.5f * scaleFactor, color);
            DrawSphereWires(center, 0.5f * scaleFactor, 4, 4, BLACK);
            break;
        }
        case ResourceType::MENDIANE: {
            float baseSize = 0.6f * scaleFactor;

            DrawCube(center, baseSize, 0.2f * scaleFactor, baseSize, color);
            DrawCubeWires(center, baseSize, 0.2f * scaleFactor, baseSize, BLACK);
            Vector3 topCenter = {center.x, center.y + 0.3f * scaleFactor, center.z};
            DrawCube(topCenter, 0.2f * scaleFactor, 0.4f * scaleFactor, 0.2f * scaleFactor, color);
            DrawCubeWires(topCenter, 0.2f * scaleFactor, 0.4f * scaleFactor, 0.2f * scaleFactor, BLACK);
            break;
        }
        case ResourceType::PHIRAS: {
            DrawSphere(center, 0.5f * scaleFactor, color);

            for (int i = 0; i < 4; i++) {
                float angleDeg = i * 90.0f;
                float angleRad = angleDeg * 3.14159f / 180.0f;
                float radius = 0.3f * scaleFactor;

                Vector3 pos = {
                    center.x + cosf(angleRad) * radius,
                    center.y + 0.2f * scaleFactor,
                    center.z + sinf(angleRad) * radius
                };

                DrawSphere(pos, 0.2f * scaleFactor, color);
            }
            break;
        }
        case ResourceType::THYSTAME: {
            DrawCube({center.x, center.y, center.z}, 0.8f * scaleFactor, 0.2f * scaleFactor, 0.2f * scaleFactor, color);
            DrawCube({center.x, center.y, center.z}, 0.2f * scaleFactor, 0.2f * scaleFactor, 0.8f * scaleFactor, color);
            DrawSphere({center.x, center.y + 0.3f * scaleFactor, center.z}, 0.3f * scaleFactor, color);

            for (int i = 0; i < 4; i++) {
                float angleDeg = i * 90.0f + 45.0f;
                float angleRad = angleDeg * 3.14159f / 180.0f;
                float radius = 0.4f * scaleFactor;

                Vector3 pos = {
                    center.x + cosf(angleRad) * radius,
                    center.y + 0.3f * scaleFactor,
                    center.z + sinf(angleRad) * radius
                };

                DrawSphere(pos, 0.15f * scaleFactor, color);
            }
            break;
        }
    }
}

ResourceType Resource::getType() const
{
    return type;
}

Vector3 Resource::getPosition() const
{
    return position;
}

Color Resource::getColor() const
{
    return color;
}

std::string Resource::getName() const
{
    return name;
}

Color Resource::getResourceColor(ResourceType type)
{
    switch (type)
    {
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

std::string Resource::getResourceName(ResourceType type)
{
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