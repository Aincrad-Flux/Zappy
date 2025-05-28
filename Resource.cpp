#include "Resource.hpp"

Resource::Resource(ResourceType resType, Vector3 pos) : type(resType), position(pos) {
    color = getResourceColor(resType);
    name = getResourceName(resType);
}

Resource::~Resource() {
    // Rien à faire
}

void Resource::draw(Vector3 worldPos, int tileSize) const {
    Vector3 center = {worldPos.x + tileSize/2.0f, 0.3f, worldPos.z + tileSize/2.0f};

    // Différentes formes 3D selon le type de ressource
    switch (type) {
        case ResourceType::FOOD:
            // Sphère pour la nourriture
            DrawSphere(center, 0.6f, color);
            DrawSphereWires(center, 0.6f, 8, 8, BLACK);
            break;

        case ResourceType::LINEMATE:
            // Cube pour linemate
            DrawCube(center, 0.8f, 0.8f, 0.8f, color);
            DrawCubeWires(center, 0.8f, 0.8f, 0.8f, BLACK);
            break;

        case ResourceType::DERAUMERE: {
            // Pyramide pour deraumere (simulée avec un cube et 4 triangles)
            DrawCube(center, 0.6f, 0.4f, 0.6f, color);
            DrawCubeWires(center, 0.6f, 0.4f, 0.6f, BLACK);

            // Dessiner un petit cube au-dessus
            Vector3 topCenter = {center.x, center.y + 0.4f, center.z};
            DrawCube(topCenter, 0.3f, 0.3f, 0.3f, color);
            DrawCubeWires(topCenter, 0.3f, 0.3f, 0.3f, BLACK);
            break;
        }

        case ResourceType::SIBUR: {
            // Diamant pour sibur (simplifié en utilisant un cube)
            float size = 0.7f;

            // Version simplifiée avec un cube
            DrawCube(center, size, size, size, color);
            DrawCubeWires(center, size, size, size, BLACK);

            // Ajouter une sphère
            DrawSphere(center, 0.5f, color);
            DrawSphereWires(center, 0.5f, 4, 4, BLACK);
            break;
        }

        case ResourceType::MENDIANE: {
            // Mendiane: une forme avec plusieurs cubes
            float baseSize = 0.6f;

            // Base plate
            DrawCube(center, baseSize, 0.2f, baseSize, color);
            DrawCubeWires(center, baseSize, 0.2f, baseSize, BLACK);

            // Pilier central
            Vector3 topCenter = {center.x, center.y + 0.3f, center.z};
            DrawCube(topCenter, 0.2f, 0.4f, 0.2f, color);
            DrawCubeWires(topCenter, 0.2f, 0.4f, 0.2f, BLACK);
            break;
        }

        case ResourceType::PHIRAS: {
            // Phiras: une forme avec plusieurs sphères
            DrawSphere(center, 0.5f, color);

            // Des sphères autour
            for (int i = 0; i < 4; i++) {
                float angleDeg = i * 90.0f;
                float angleRad = angleDeg * 3.14159f / 180.0f;
                float radius = 0.3f;

                Vector3 pos = {
                    center.x + cosf(angleRad) * radius,
                    center.y + 0.2f,
                    center.z + sinf(angleRad) * radius
                };

                DrawSphere(pos, 0.2f, color);
            }
            break;
        }

        case ResourceType::THYSTAME: {
            // Thystame: une forme complexe avec cubes et sphères
            // Base en croix
            DrawCube({center.x, center.y, center.z}, 0.8f, 0.2f, 0.2f, color);
            DrawCube({center.x, center.y, center.z}, 0.2f, 0.2f, 0.8f, color);

            // Sphère centrale
            DrawSphere({center.x, center.y + 0.3f, center.z}, 0.3f, color);

            // Quelques petites sphères décoratives
            for (int i = 0; i < 4; i++) {
                float angleDeg = i * 90.0f + 45.0f;
                float angleRad = angleDeg * 3.14159f / 180.0f;
                float radius = 0.4f;

                Vector3 pos = {
                    center.x + cosf(angleRad) * radius,
                    center.y + 0.3f,
                    center.z + sinf(angleRad) * radius
                };

                DrawSphere(pos, 0.15f, color);
            }
            break;
        }
    }
}

ResourceType Resource::getType() const {
    return type;
}

Vector3 Resource::getPosition() const {
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