/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** Inventory.cpp
*/

#include "Inventory.hpp"

Inventory::Inventory(int food, int linemate, int deraumere,
                     int sibur, int mendiane, int phiras, int thystame)
    : m_food(food), m_linemate(linemate), m_deraumere(deraumere),
      m_sibur(sibur), m_mendiane(mendiane), m_phiras(phiras), m_thystame(thystame)
{
}

Inventory::~Inventory()
{
}

bool Inventory::removeFood(int amount) {
    if (m_food >= amount) {
        m_food -= amount;
        return true;
    }
    return false;
}

bool Inventory::removeLinemate(int amount) {
    if (m_linemate >= amount) {
        m_linemate -= amount;
        return true;
    }
    return false;
}

bool Inventory::removeDeraumere(int amount) {
    if (m_deraumere >= amount) {
        m_deraumere -= amount;
        return true;
    }
    return false;
}

bool Inventory::removeSibur(int amount) {
    if (m_sibur >= amount) {
        m_sibur -= amount;
        return true;
    }
    return false;
}

bool Inventory::removeMendiane(int amount) {
    if (m_mendiane >= amount) {
        m_mendiane -= amount;
        return true;
    }
    return false;
}

bool Inventory::removePhiras(int amount) {
    if (m_phiras >= amount) {
        m_phiras -= amount;
        return true;
    }
    return false;
}

bool Inventory::removeThystame(int amount) {
    if (m_thystame >= amount) {
        m_thystame -= amount;
        return true;
    }
    return false;
}
