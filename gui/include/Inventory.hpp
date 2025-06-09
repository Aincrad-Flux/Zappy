/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** Inventory.hpp
*/

#pragma once

/**
 * @class Inventory
 * @brief Contains the count of resources a player has
 */
class Inventory {
private:
    int m_food;       ///< Amount of food
    int m_linemate;   ///< Amount of linemate
    int m_deraumere;  ///< Amount of deraumere
    int m_sibur;      ///< Amount of sibur
    int m_mendiane;   ///< Amount of mendiane
    int m_phiras;     ///< Amount of phiras
    int m_thystame;   ///< Amount of thystame

public:
    /**
     * @brief Constructor for Inventory
     * @param food Amount of food
     * @param linemate Amount of linemate
     * @param deraumere Amount of deraumere
     * @param sibur Amount of sibur
     * @param mendiane Amount of mendiane
     * @param phiras Amount of phiras
     * @param thystame Amount of thystame
     */
    Inventory(int food = 0, int linemate = 0, int deraumere = 0,
              int sibur = 0, int mendiane = 0, int phiras = 0, int thystame = 0);

    /**
     * @brief Destructor for Inventory
     */
    ~Inventory();

    // Getters
    int getFood() const { return m_food; }
    int getLinemate() const { return m_linemate; }
    int getDeraumere() const { return m_deraumere; }
    int getSibur() const { return m_sibur; }
    int getMendiane() const { return m_mendiane; }
    int getPhiras() const { return m_phiras; }
    int getThystame() const { return m_thystame; }

    // Setters
    void setFood(int value) { m_food = value; }
    void setLinemate(int value) { m_linemate = value; }
    void setDeraumere(int value) { m_deraumere = value; }
    void setSibur(int value) { m_sibur = value; }
    void setMendiane(int value) { m_mendiane = value; }
    void setPhiras(int value) { m_phiras = value; }
    void setThystame(int value) { m_thystame = value; }

    // Add resource methods
    void addFood(int amount) { m_food += amount; }
    void addLinemate(int amount) { m_linemate += amount; }
    void addDeraumere(int amount) { m_deraumere += amount; }
    void addSibur(int amount) { m_sibur += amount; }
    void addMendiane(int amount) { m_mendiane += amount; }
    void addPhiras(int amount) { m_phiras += amount; }
    void addThystame(int amount) { m_thystame += amount; }

    // Remove resource methods (with validation)
    bool removeFood(int amount);
    bool removeLinemate(int amount);
    bool removeDeraumere(int amount);
    bool removeSibur(int amount);
    bool removeMendiane(int amount);
    bool removePhiras(int amount);
    bool removeThystame(int amount);
};
