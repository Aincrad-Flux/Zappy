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
#include <unordered_map>
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
    std::unordered_map<std::string, Color> teamColors; ///< Map of team names to colors
    bool is3DMode;                    ///< Flag indicating if we're in 3D mode
    Vector2 selectedTile;             ///< Coordinates of the selected tile
    int tileResources[7];             ///< Resources on the selected tile [food, linemate, deraumere, sibur, mendiane, phiras, thystame]
    bool showTileInfo;                ///< Flag to toggle tile information panel visibility

    /**
     * @brief Draws the player information panel
     */
    void drawPlayerInfo();

    /**
     * @brief Draws the tile information panel
     */
    void drawTileInfo();

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

    /**
     * @brief Sets the view mode (2D or 3D)
     * @param mode True for 3D mode, false for 2D mode
     */
    void set3DMode(bool mode);

    /**
     * @brief Gets the current view mode
     * @return True if 3D mode, false if 2D mode
     */
    bool getIs3DMode() const;

    /**
     * @brief Shows a game over message
     * @param message The game over message to display
     */
    void showGameOverMessage(const std::string& message);

    /**
     * @brief Shows a message from the server
     * @param message The server message to display
     */
    void showServerMessage(const std::string& message);

    /**
     * @brief Sets the color for a team
     * @param teamName Name of the team
     * @param color Color to associate with the team
     */
    void setTeamColor(const std::string& teamName, Color color);

    /**
     * @brief Sets the selected tile coordinates
     * @param tile The coordinates of the selected tile
     * @param resources Array of resources on the tile [food, linemate, deraumere, sibur, mendiane, phiras, thystame]
     */
    void setSelectedTile(const Vector2& tile, const int resources[7]);

    /**
     * @brief Toggles the tile information panel visibility
     */
    void toggleTileInfo();

private:
    std::string getDirectionString(PlayerDirection direction);
    std::string gameOverMessage;
    std::string serverMessage;
    float messageDisplayTime;
};