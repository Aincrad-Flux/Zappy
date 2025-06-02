/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** Game.hpp
*/

#pragma once

#include <raylib.h>
#include <vector>
#include <string>
#include <memory>
#include "Map.hpp"
#include "Player.hpp"
#include "Resource.hpp"
#include "UI.hpp"

/**
 * @class Game
 * @brief Main game class that handles the game loop and state
 *
 * This class manages the overall game state, entities, rendering, and input handling.
 */
class Game {
private:
    int screenWidth;                     ///< Width of the screen
    int screenHeight;                    ///< Height of the screen
    std::unique_ptr<Map> gameMap;        ///< Game world map
    std::unique_ptr<UI> gameUI;          ///< User interface
    std::vector<Player> players;         ///< List of players
    std::vector<Resource> resources;     ///< List of resources
    Camera3D camera;                     ///< 3D camera for world view
    bool running;                        ///< Flag indicating if the game is running

    /**
     * @brief Processes user input
     */
    void handleInput();

    /**
     * @brief Updates game state
     */
    void update();

    /**
     * @brief Renders the game
     */
    void render();

    /**
     * @brief Initializes test data for the game
     */
    void initializeMockData();

    /**
     * @brief Centers the camera on the map
     */
    void centerCamera();

public:
    /**
     * @brief Constructor for Game
     * @param width Width of the game window
     * @param height Height of the game window
     */
    Game(int width = 1200, int height = 800);

    /**
     * @brief Destructor for Game
     */
    ~Game();

    /**
     * @brief Runs the main game loop
     */
    void run();

    /**
     * @brief Safely shuts down the game
     */
    void shutdown();
};