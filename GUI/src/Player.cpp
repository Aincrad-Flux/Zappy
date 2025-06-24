/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** Player.cpp
*/

#include "Player.hpp"
#include <cmath>
#include "Logger.hpp"

Player::Player(int playerId, const std::string& team, Vector3 pos, Color color)
    : id(playerId), teamName(team), position(pos), team(team), direction(PlayerDirection::NORTH),
      level(1), inventory(10, 0, 0, 0, 0, 0, 0), teamColor(color), isAlive(true),
      lifeTime(1260.0f), isIncanting(false), isBroadcasting(false), broadcastTimer(0.0f),
      isLevelingUp(false), levelUpTimer(0.0f), previousLevel(1)
{
    Logger::getInstance().info("Player created: ID " + std::to_string(id) + " from team " + team +
        " at position (" + std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z) + ")");
}

Player::~Player() {
    Logger::getInstance().debug("Player destroyed: ID " + std::to_string(id) + " from team " + team);
}

void Player::draw(Vector3 worldPos, int tileSize) const
{
    if (!isAlive) return;

    Vector3 center = {worldPos.x + tileSize/2.0f, 0.5f, worldPos.z + tileSize/2.0f};
    float radius = tileSize * 0.3f;
    float height = tileSize * 0.6f;

    Color playerColor = teamColor;

    // Effet de pulsation pour l'incantation
    if (isIncanting) {
        float pulse = sinf(GetTime() * 5.0f) * 0.3f + 0.7f;
        playerColor = ColorAlpha(playerColor, pulse);
    }

    // Effet d'animation pour le level-up
    if (isLevelingUp) {
        // Créer un effet visuel spectaculaire pour le level-up
        float animationProgress = 1.0f - (levelUpTimer / 3.0f); // 0.0 (début) à 1.0 (fin)
        float levelPulse = sinf(GetTime() * 10.0f) * 0.5f + 0.5f;

        // Pilier de lumière montant vers le haut
        float pillarHeight = height * (2.0f + 3.0f * animationProgress);
        float pillarAlpha = 1.0f - animationProgress;  // Disparaît progressivement

        // Dessiner un pilier de lumière
        Color pillarColor = ColorAlpha(GOLD, pillarAlpha * (0.7f + levelPulse * 0.3f));
        DrawCylinder(
            {center.x, center.y, center.z},                     // base center
            radius * 0.3f * (1.0f - animationProgress * 0.5f),  // base radius
            radius * 0.1f,                                      // top radius
            pillarHeight,                                       // height
            8,                                                  // sides
            pillarColor                                         // color
        );

        // Cercles d'énergie montant le long du pilier
        for (int i = 0; i < 5; i++) {
            float circleHeight = center.y + (animationProgress * pillarHeight * 0.8f) +
                               sinf(GetTime() * 5.0f + i * 0.5f) * pillarHeight * 0.2f;
            // Dessiner un anneau en 3D (DrawTorus n'est pas disponible dans cette version de Raylib)
            float innerRadius = radius * (0.5f + levelPulse * 0.2f);
            float outerRadius = radius * (0.6f + levelPulse * 0.3f);
            Color ringColor = ColorAlpha(GOLD, (1.0f - animationProgress) * 0.7f);

            // Dessiner plusieurs cercles pour créer l'effet d'un anneau
            const int segments = 16;
            for (int j = 0; j < segments; j++) {
                float angle1 = ((float)j / segments) * 2.0f * PI;
                float angle2 = ((float)(j+1) / segments) * 2.0f * PI;

                Vector3 p1Inner = {
                    center.x + innerRadius * cosf(angle1),
                    circleHeight,
                    center.z + innerRadius * sinf(angle1)
                };
                Vector3 p2Inner = {
                    center.x + innerRadius * cosf(angle2),
                    circleHeight,
                    center.z + innerRadius * sinf(angle2)
                };
                Vector3 p1Outer = {
                    center.x + outerRadius * cosf(angle1),
                    circleHeight,
                    center.z + outerRadius * sinf(angle1)
                };
                Vector3 p2Outer = {
                    center.x + outerRadius * cosf(angle2),
                    circleHeight,
                    center.z + outerRadius * sinf(angle2)
                };

                DrawLine3D(p1Inner, p2Inner, ringColor);
                DrawLine3D(p1Outer, p2Outer, ringColor);
            }
        }

        // Faire briller le joueur
        playerColor = ColorAlpha(ColorBrightness(teamColor, 1.0f + levelPulse * 0.5f), 1.0f);

        // Particules s'élevant autour du joueur
        for (int i = 0; i < 10; i++) {
            float angle = (i / 10.0f) * 2.0f * PI + GetTime();
            float distance = radius * (1.0f + animationProgress * 0.5f + sinf(GetTime() * 2.0f + i) * 0.2f);
            float particleHeight = center.y + height * (0.3f + animationProgress * 1.5f + sin(GetTime() * 3.0f + i * 0.2f) * 0.3f);

            Vector3 particlePos = {
                center.x + cosf(angle) * distance,
                particleHeight,
                center.z + sinf(angle) * distance
            };

            float particleSize = tileSize * 0.05f * (1.0f - animationProgress * 0.7f + levelPulse * 0.3f);
            DrawSphere(particlePos, particleSize, ColorAlpha(GOLD, (1.0f - animationProgress) * 0.9f));
        }
    }

    // Effet de clignotement pour le broadcast
    if (isBroadcasting) {
        // Clignotement rapide (10 Hz) avec une halo blanc
        float blink = sinf(GetTime() * 10.0f);

        // Dessiner un halo lumineux autour du joueur
        float haloRadius = radius * (1.3f + blink * 0.2f);
        DrawSphere(center, haloRadius, ColorAlpha(WHITE, 0.3f));

        // Dessiner des cercles d'onde qui se propagent
        for (int i = 1; i <= 3; i++) {
            float waveRadius = radius * (1.5f + i * 0.5f * broadcastTimer);
            DrawSphereWires(center, waveRadius, 8, 8, ColorAlpha(WHITE, 0.8f - i * 0.25f));
        }

        // Rendre le joueur légèrement plus brillant
        playerColor = ColorBrightness(playerColor, 0.3f + blink * 0.2f);
    }

    DrawCylinder(center, radius, radius, height, 8, playerColor);
    DrawCylinderWires(center, radius, radius, height, 8, BLACK);

    Vector3 headPos = {center.x, center.y + height/2.0f + radius*0.5f, center.z};
    DrawSphere(headPos, radius*0.5f, playerColor);

    Vector3 dirOffset = {0, 0, 0};
    switch (direction) {
        case PlayerDirection::NORTH: dirOffset = {0, 0, -radius * 0.9f}; break;
        case PlayerDirection::EAST:  dirOffset = {radius * 0.9f, 0, 0}; break;
        case PlayerDirection::SOUTH: dirOffset = {0, 0, radius * 0.9f}; break;
        case PlayerDirection::WEST:  dirOffset = {-radius * 0.9f, 0, 0}; break;
    }

    Vector3 frontPos = {
        center.x + dirOffset.x,
        center.y,
        center.z + dirOffset.z
    };
    DrawSphere(frontPos, radius * 0.2f, WHITE);

    Vector3 levelPos = {center.x, center.y + height + radius, center.z};
    DrawSphere(levelPos, radius * 0.3f, BLACK);

    for(int i = 0; i < level && i < 5; i++) {
        DrawSphere({levelPos.x + (i-2)*radius*0.4f, levelPos.y, levelPos.z + radius*0.4f},
                   radius * 0.15f, WHITE);
    }

    float lifePercent = lifeTime / 1260.0f;
    Color lifeColor = (lifePercent > 0.5f) ? GREEN : (lifePercent > 0.25f) ? YELLOW : RED;

    Vector3 lifeBarStart = {center.x - tileSize/2.0f, center.y + height + radius*1.5f, center.z};
    Vector3 lifeBarEnd = {center.x - tileSize/2.0f + tileSize * lifePercent, center.y + height + radius*1.5f, center.z};
    DrawLine3D(lifeBarStart, lifeBarEnd, lifeColor);
}

void Player::update(float deltaTime)
{
    if (!isAlive) return;

    lifeTime -= deltaTime;
    if (lifeTime <= 0) {
        Logger::getInstance().info("Player " + std::to_string(id) + " from team " + team + " died of starvation");
        isAlive = false;
    }

    // Gestion du timer de broadcast
    if (isBroadcasting) {
        broadcastTimer -= deltaTime;
        if (broadcastTimer <= 0) {
            isBroadcasting = false;
            broadcastTimer = 0;
        }
    }

    // Gestion du timer de l'animation de level-up
    if (isLevelingUp) {
        levelUpTimer -= deltaTime;
        if (levelUpTimer <= 0) {
            isLevelingUp = false;
            levelUpTimer = 0;
        }
    }

    static float foodTimer = 0;
    foodTimer += deltaTime;
    if (foodTimer >= 126.0f) {
        if (inventory.getFood() > 0) {
            inventory.removeFood(1);
            float oldLifeTime = lifeTime;
            lifeTime = std::min(lifeTime + 126.0f, 1260.0f);
            Logger::getInstance().debug("Player " + std::to_string(id) + " consumed food, life increased from " +
                std::to_string(oldLifeTime) + " to " + std::to_string(lifeTime));
        } else {
            Logger::getInstance().debug("Player " + std::to_string(id) + " has no food to consume");
        }
        foodTimer = 0;
    }
}

void Player::move(Vector3 newPos)
{
    Logger::getInstance().debug("Player " + std::to_string(id) + " moved from (" +
        std::to_string(position.x) + "," + std::to_string(position.y) + "," + std::to_string(position.z) + ") to (" +
        std::to_string(newPos.x) + "," + std::to_string(newPos.y) + "," + std::to_string(newPos.z) + ")");
    position = newPos;
}

void Player::setPosition(Vector3 newPos)
{
    if (position.x != newPos.x || position.y != newPos.y || position.z != newPos.z) {
        Logger::getInstance().debug("Player " + std::to_string(id) + " position set from (" +
            std::to_string(position.x) + "," + std::to_string(position.y) + "," + std::to_string(position.z) + ") to (" +
            std::to_string(newPos.x) + "," + std::to_string(newPos.y) + "," + std::to_string(newPos.z) + ")");
    }
    position = newPos;
}

void Player::rotate(PlayerDirection newDir)
{
    direction = newDir;
}

void Player::setDirection(PlayerDirection newDir)
{
    direction = newDir;
}

void Player::setLevel(int newLevel)
{
    if (level != newLevel) {
        Logger::getInstance().info("Player " + std::to_string(id) + " level changed from " +
            std::to_string(level) + " to " + std::to_string(newLevel));

        if (newLevel > level) {
            // Only start animation if leveling up (not down)
            startLevelUpAnimation(newLevel);
        } else {
            level = newLevel;
        }
    }
}

void Player::setTeam(const std::string& newTeam)
{
    if (team != newTeam) {
        Logger::getInstance().info("Player " + std::to_string(id) + " team changed from '" +
            team + "' to '" + newTeam + "'");
    }
    teamName = newTeam;
    team = newTeam;
}

void Player::setIncanting(bool incanting)
{
    if (isIncanting != incanting) {
        Logger::getInstance().info("Player " + std::to_string(id) + (incanting ? " started" : " stopped") + " incantation");
    }
    isIncanting = incanting;
}


int Player::getId() const
{
    return id;
}

std::string Player::getTeamName() const
{
    return teamName;
}

Vector3 Player::getPosition() const
{
    return position;
}

PlayerDirection Player::getDirection() const
{
    return direction;
}

int Player::getLevel() const
{
    return level;
}

Inventory& Player::getInventory()
{
    return inventory;
}

Inventory Player::getInventory() const
{
    return inventory;
}

bool Player::getIsAlive() const
{
    return isAlive;
}

float Player::getLifeTime() const
{
    return lifeTime;
}

Color Player::getTeamColor() const
{
    return teamColor;
}

bool Player::getIsIncanting() const
{
    return isIncanting;
}

void Player::setColor(Color color)
{
    teamColor = color;
}

void Player::setIsAlive(bool alive)
{
    if (isAlive != alive) {
        if (!alive) {
            Logger::getInstance().info("Player " + std::to_string(id) + " from team " + team + " died");
        } else {
            Logger::getInstance().info("Player " + std::to_string(id) + " from team " + team + " resurrected");
        }
    }
    isAlive = alive;
}

void Player::setInventory(const Inventory& inv)
{
    inventory = inv;
}

void Player::addToInventory(int resourceType, int amount)
{
    switch (resourceType) {
        case 0: inventory.addFood(amount); break;
        case 1: inventory.addLinemate(amount); break;
        case 2: inventory.addDeraumere(amount); break;
        case 3: inventory.addSibur(amount); break;
        case 4: inventory.addMendiane(amount); break;
        case 5: inventory.addPhiras(amount); break;
        case 6: inventory.addThystame(amount); break;
    }
}

void Player::startBroadcasting()
{
    Logger::getInstance().debug("Player " + std::to_string(id) + " started broadcasting a message");
    isBroadcasting = true;
    broadcastTimer = 1.0f; // Animation dure 1 seconde
}

void Player::startLevelUpAnimation(int newLevel)
{
    if (!isAlive || level == newLevel) return;

    previousLevel = level;
    level = newLevel;
    isLevelingUp = true;
    levelUpTimer = 3.0f; // Animation duration in seconds

    Logger::getInstance().info("Player " + std::to_string(id) + " starting level-up animation from level " +
                               std::to_string(previousLevel) + " to " + std::to_string(level));
}

bool Player::getIsBroadcasting() const
{
    return isBroadcasting;
}

bool Player::getIsLevelingUp() const
{
    return isLevelingUp;
}

int Player::getPreviousLevel() const
{
    return previousLevel;
}