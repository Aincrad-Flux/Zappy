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
      lifeTime(1260.0f), isIncanting(false)
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
    if (isIncanting) {
        float pulse = sinf(GetTime() * 5.0f) * 0.3f + 0.7f;
        playerColor = ColorAlpha(playerColor, pulse);
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
    }
    level = newLevel;
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