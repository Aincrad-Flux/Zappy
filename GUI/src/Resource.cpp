/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** Resource.cpp
*/

#include "Resource.hpp"
#include <cmath>

Resource::Resource(ResourceType resType, Vector3 pos) : type(resType), position(pos), count(1)
{
    color = getResourceColor(resType);
    name = getResourceName(resType);
}

Resource::~Resource() {}

void Resource::draw(Vector3 worldPos, int tileSize) const
{
    int hash = (static_cast<int>(position.x) * 17 + static_cast<int>(position.z) * 31 + static_cast<int>(type)) % 20;
    float edgeOffset = tileSize * 0.2f;
    Vector3 center;
    if (hash < 5) {
        // Bord haut
        center = {
            worldPos.x + edgeOffset + (hash % 5) * ((tileSize - 2 * edgeOffset) / 4),
            0.3f,
            worldPos.z + edgeOffset
        };
    } else if (hash < 10) {
        // Bord droit
        center = {
            worldPos.x + tileSize - edgeOffset,
            0.3f,
            worldPos.z + edgeOffset + (hash % 5) * ((tileSize - 2 * edgeOffset) / 4)
        };
    } else if (hash < 15) {
        // Bord bas
        center = {
            worldPos.x + edgeOffset + (hash % 5) * ((tileSize - 2 * edgeOffset) / 4),
            0.3f,
            worldPos.z + tileSize - edgeOffset
        };
    } else {
        // Bord gauche
        center = {
            worldPos.x + edgeOffset,
            0.3f,
            worldPos.z + edgeOffset + (hash % 5) * ((tileSize - 2 * edgeOffset) / 4)
        };
    }

    const float scaleFactor = 7.5f;

    switch (type)
    {
        case ResourceType::FOOD:
            DrawSphere(center, 0.4f * scaleFactor, color);
            DrawSphereWires(center, 0.4f * scaleFactor, 8, 8, BLACK);
            break;

        case ResourceType::LINEMATE:
            DrawCube(center, 0.5f * scaleFactor, 0.5f * scaleFactor, 0.5f * scaleFactor, color);
            DrawCubeWires(center, 0.5f * scaleFactor, 0.5f * scaleFactor, 0.5f * scaleFactor, BLACK);
            break;

        case ResourceType::DERAUMERE: {
            DrawCube(center, 0.4f * scaleFactor, 0.3f * scaleFactor, 0.4f * scaleFactor, color);
            DrawCubeWires(center, 0.4f * scaleFactor, 0.3f * scaleFactor, 0.4f * scaleFactor, BLACK);

            Vector3 topCenter = {center.x, center.y + 0.3f * scaleFactor, center.z};
            DrawCube(topCenter, 0.2f * scaleFactor, 0.2f * scaleFactor, 0.2f * scaleFactor, color);
            DrawCubeWires(topCenter, 0.2f * scaleFactor, 0.2f * scaleFactor, 0.2f * scaleFactor, BLACK);
            break;
        }
        case ResourceType::SIBUR: {
            float size = 0.45f * scaleFactor;

            DrawCube(center, size, size, size, color);
            DrawCubeWires(center, size, size, size, BLACK);
            DrawSphere(center, 0.35f * scaleFactor, color);
            DrawSphereWires(center, 0.35f * scaleFactor, 4, 4, BLACK);
            break;
        }
        case ResourceType::MENDIANE: {
            float baseSize = 0.4f * scaleFactor;

            DrawCube(center, baseSize, 0.15f * scaleFactor, baseSize, color);
            DrawCubeWires(center, baseSize, 0.15f * scaleFactor, baseSize, BLACK);
            Vector3 topCenter = {center.x, center.y + 0.2f * scaleFactor, center.z};
            DrawCube(topCenter, 0.15f * scaleFactor, 0.3f * scaleFactor, 0.15f * scaleFactor, color);
            DrawCubeWires(topCenter, 0.15f * scaleFactor, 0.3f * scaleFactor, 0.15f * scaleFactor, BLACK);
            break;
        }
        case ResourceType::PHIRAS: {
            DrawSphere(center, 0.3f * scaleFactor, color);

            for (int i = 0; i < 4; i++) {
                float angleDeg = i * 90.0f;
                float angleRad = angleDeg * 3.14159f / 180.0f;
                float radius = 0.2f * scaleFactor;

                Vector3 pos = {
                    center.x + cosf(angleRad) * radius,
                    center.y + 0.15f * scaleFactor,
                    center.z + sinf(angleRad) * radius
                };

                DrawSphere(pos, 0.12f * scaleFactor, color);
            }
            break;
        }
        case ResourceType::THYSTAME: {
            DrawCube({center.x, center.y, center.z}, 0.5f * scaleFactor, 0.15f * scaleFactor, 0.15f * scaleFactor, color);
            DrawCube({center.x, center.y, center.z}, 0.15f * scaleFactor, 0.15f * scaleFactor, 0.5f * scaleFactor, color);
            DrawSphere({center.x, center.y + 0.2f * scaleFactor, center.z}, 0.2f * scaleFactor, color);

            for (int i = 0; i < 4; i++) {
                float angleDeg = i * 90.0f + 45.0f;
                float angleRad = angleDeg * 3.14159f / 180.0f;
                float radius = 0.25f * scaleFactor;

                Vector3 pos = {
                    center.x + cosf(angleRad) * radius,
                    center.y + 0.2f * scaleFactor,
                    center.z + sinf(angleRad) * radius
                };

                DrawSphere(pos, 0.1f * scaleFactor, color);
            }
            break;
        }
    }

    // Ajouter un indicateur visuel de quantité pour les ressources multiples
    if (count > 3) {
        // Créer un halo autour de la ressource pour indiquer qu'il y en a plusieurs
        Color highlightColor = WHITE;
        highlightColor.a = 100; // Semi-transparent

        // Rendre la ressource légèrement plus grande et plus brillante pour indiquer quantité
        float pulseIntensity = 0.1f * sinf(GetTime() * 2.0f) + 1.1f; // Effet de pulsation

        // Dessiner un halo ou une sphère additionnelle autour de la ressource
        DrawSphere(center, 0.6f * scaleFactor * pulseIntensity, highlightColor);
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

int Resource::getCount() const
{
    return count;
}

void Resource::setCount(int newCount)
{
    count = newCount;
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