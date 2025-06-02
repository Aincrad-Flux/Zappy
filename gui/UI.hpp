/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** UI.hpp
*/

#pragma once

#include <raylib.h>
#include <vector>
#include <string>
#include "Player.hpp"

/**
 * @class UI
 * @brief Class handling all UI elements and display for the Zappy GUI
 *
 * This class manages the user interface elements including menus, player information,
 * team statistics, and help screens.
 */
class UI {
private:
    int screenWidth;                  ///< Current screen width
    int screenHeight;                 ///< Current screen height
    Font font;                        ///< Font used for UI text
    Player* selectedPlayer;           ///< Pointer to the currently selected player
    bool showPlayerInfo;              ///< Flag to toggle player information panel visibility
    bool showTeamStats;               ///< Flag to toggle team statistics panel visibility
    bool showMenu;                    ///< Flag to toggle menu visibility
    bool showHelp;                    ///< Flag to toggle help screen visibility
    std::vector<std::string> teams;   ///< List of team names

    /**
     * @brief Draws the player information panel
     */
    void drawPlayerInfo();

    /**
     * @brief Draws the team statistics panel
     */
    void drawTeamStats();

    /**
     * @brief Draws the resource legend
     */
    void drawResourceLegend();

    /**
     * @brief Draws the game statistics
     */
    void drawGameStats();

    /**
     * @brief Draws the menu with controls
     */
    void drawMenu();

    /**
     * @brief Draws the help screen
     */
    void drawHelp();

public:
    /**
     * @brief Constructor for UI class
     * @param width Initial screen width
     * @param height Initial screen height
     */
    UI(int width, int height);

    /**
     * @brief Destructor for UI class
     */
    ~UI();

    /**
     * @brief Main draw function that handles all UI elements
     * @param players Vector of all players in the game
     */
    void draw(const std::vector<Player>& players);

    /**
     * @brief Handles user input for UI interactions
     */
    void handleInput();

    /**
     * @brief Sets the currently selected player
     * @param player Pointer to the selected player
     */
    void setSelectedPlayer(Player* player);

    /**
     * @brief Adds a team to the list of teams
     * @param teamName Name of the team to add
     */
    void addTeam(const std::string& teamName);

    /**
     * @brief Toggles the player information panel visibility
     */
    void togglePlayerInfo();

    /**
     * @brief Toggles the team statistics panel visibility
     */
    void toggleTeamStats();

    /**
     * @brief Toggles the menu visibility
     */
    void toggleMenu();

    /**
     * @brief Toggles the help screen visibility
     */
    void toggleHelp();
};