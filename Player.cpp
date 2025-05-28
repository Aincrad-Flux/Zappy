#include "Player.hpp"
#include <cmath>

Player::Player(int playerId, const std::string& team, Vector2 pos, Color color)
    : id(playerId), teamName(team), position(pos), direction(PlayerDirection::NORTH),
      level(1), teamColor(color), isAlive(true), lifeTime(1260.0f), isIncanting(false) {

    // Initialiser l'inventaire
    inventory = {10, 0, 0, 0, 0, 0, 0}; // 10 unités de nourriture au début
}

Player::~Player() {
    // Rien à faire
}

void Player::draw(Vector2 worldPos, int tileSize) const {
    if (!isAlive) return;

    Vector2 center = {worldPos.x + tileSize/2.0f, worldPos.y + tileSize/2.0f};
    float radius = tileSize * 0.3f;

    // Dessiner le corps du joueur
    Color playerColor = teamColor;
    if (isIncanting) {
        // Effet de pulsation pour l'incantation
        float pulse = sinf(GetTime() * 5.0f) * 0.3f + 0.7f;
        playerColor = ColorAlpha(playerColor, pulse);
    }

    DrawCircleV(center, radius, playerColor);
    DrawCircleLinesV(center, radius, BLACK);

    // Dessiner l'indicateur de direction
    Vector2 dirOffset = {0, 0};
    switch (direction) {
        case PlayerDirection::NORTH: dirOffset = {0, -radius * 0.7f}; break;
        case PlayerDirection::EAST:  dirOffset = {radius * 0.7f, 0}; break;
        case PlayerDirection::SOUTH: dirOffset = {0, radius * 0.7f}; break;
        case PlayerDirection::WEST:  dirOffset = {-radius * 0.7f, 0}; break;
    }

    Vector2 dirPos = {center.x + dirOffset.x, center.y + dirOffset.y};
    DrawCircleV(dirPos, 3, WHITE);

    // Dessiner le niveau
    DrawText(TextFormat("%d", level), (int)(center.x - 5), (int)(center.y - 8), 16, BLACK);

    // Dessiner la barre de vie
    float lifePercent = lifeTime / 1260.0f;
    Rectangle lifeBar = {worldPos.x, worldPos.y - 8, tileSize * lifePercent, 4};
    Color lifeColor = (lifePercent > 0.5f) ? GREEN : (lifePercent > 0.25f) ? YELLOW : RED;
    DrawRectangleRec(lifeBar, lifeColor);
    DrawRectangleLinesEx({worldPos.x, worldPos.y - 8, (float)tileSize, 4}, 1, BLACK);
}

void Player::update(float deltaTime) {
    if (!isAlive) return;

    // Décrémenter le temps de vie
    lifeTime -= deltaTime;
    if (lifeTime <= 0) {
        isAlive = false;
    }

    // Consommer de la nourriture automatiquement
    static float foodTimer = 0;
    foodTimer += deltaTime;
    if (foodTimer >= 126.0f) { // Une unité de nourriture dure 126 unités de temps
        if (inventory.food > 0) {
            inventory.food--;
            lifeTime = std::min(lifeTime + 126.0f, 1260.0f);
        }
        foodTimer = 0;
    }
}

void Player::move(Vector2 newPos) {
    position = newPos;
}

void Player::rotate(PlayerDirection newDir) {
    direction = newDir;
}

void Player::setLevel(int newLevel) {
    level = newLevel;
}

void Player::setIncanting(bool incanting) {
    isIncanting = incanting;
}

// Getters
int Player::getId() const { return id; }
std::string Player::getTeamName() const { return teamName; }
Vector2 Player::getPosition() const { return position; }
PlayerDirection Player::getDirection() const { return direction; }
int Player::getLevel() const { return level; }
Inventory Player::getInventory() const { return inventory; }
bool Player::getIsAlive() const { return isAlive; }
float Player::getLifeTime() const { return lifeTime; }
Color Player::getTeamColor() const { return teamColor; }
bool Player::getIsIncanting() const { return isIncanting; }

void Player::setInventory(const Inventory& inv) {
    inventory = inv;
}

void Player::addToInventory(int resourceType, int amount) {
    switch (resourceType) {
        case 0: inventory.food += amount; break;
        case 1: inventory.linemate += amount; break;
        case 2: inventory.deraumere += amount; break;
        case 3: inventory.sibur += amount; break;
        case 4: inventory.mendiane += amount; break;
        case 5: inventory.phiras += amount; break;
        case 6: inventory.thystame += amount; break;
    }
}