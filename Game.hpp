#pragma once

#include <raylib.h>
#include <vector>
#include <string>
#include <memory>
#include "Map.hpp"
#include "Player.hpp"
#include "Resource.hpp"
#include "UI.hpp"

class Game {
private:
    int screenWidth;
    int screenHeight;
    std::unique_ptr<Map> gameMap;
    std::unique_ptr<UI> gameUI;
    std::vector<Player> players;
    std::vector<Resource> resources;
    Camera3D camera;
    bool running;

    void handleInput();
    void update();
    void render();
    void initializeMockData();
    void centerCamera(); // Nouvelle méthode pour centrer la caméra

public:
    Game(int width = 1200, int height = 800);
    ~Game();

    void run();
    void shutdown();
};