#include "Game.hpp"
#include <iostream>
#include <random>
#include <cmath> 

Game::Game(int width, int height) : screenWidth(width), screenHeight(height), running(false) {
    InitWindow(screenWidth, screenHeight, "Zappy GUI - Raylib");
    SetTargetFPS(60);

    gameMap = std::make_unique<Map>(20, 15, 32);
    gameUI = std::make_unique<UI>(screenWidth, screenHeight);

    // Initialize camera
    camera.target = { (float)gameMap->getWidth() * gameMap->getTileSize() / 2.0f,
                      (float)gameMap->getHeight() * gameMap->getTileSize() / 2.0f };
    camera.offset = { (float)screenWidth / 2.0f, (float)screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    initializeMockData();
}

Game::~Game() {
    // Destructors automatiques pour les unique_ptr
}

void Game::initializeMockData() {
    // Créer quelques équipes de test
    gameUI->addTeam("Team Alpha");
    gameUI->addTeam("Team Beta");
    gameUI->addTeam("Team Gamma");

    // Créer quelques joueurs de test
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> posDist(0, std::min(gameMap->getWidth(), gameMap->getHeight()) - 1);

    Color teamColors[] = {RED, BLUE, GREEN, YELLOW, PURPLE, ORANGE};
    std::string teamNames[] = {"Alpha", "Beta", "Gamma"};

    for (int i = 0; i < 6; i++) {
        Vector2 pos = {(float)posDist(gen), (float)posDist(gen)};
        players.emplace_back(i, teamNames[i % 3], pos, teamColors[i % 6]);

        // Mettre à jour la carte
        gameMap->setTilePlayer((int)pos.x, (int)pos.y, 1);
    }

    // Créer quelques ressources de test
    std::uniform_int_distribution<> resourceDist(0, 6);
    for (int i = 0; i < 30; i++) {
        Vector2 pos = {(float)posDist(gen), (float)posDist(gen)};
        ResourceType type = (ResourceType)resourceDist(gen);
        resources.emplace_back(type, pos);

        // Mettre à jour la carte
        gameMap->setTileResource((int)pos.x, (int)pos.y, (int)type);
    }
}

void Game::handleInput() {
    // Contrôles de la caméra
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) camera.target.x += 5.0f;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) camera.target.x -= 5.0f;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) camera.target.y += 5.0f;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) camera.target.y -= 5.0f;

    // Zoom
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        camera.zoom += wheel * 0.1f;
        if (camera.zoom < 0.5f) camera.zoom = 0.5f;
        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
    }

    // Interface
    if (IsKeyPressed(KEY_I)) gameUI->togglePlayerInfo();
    if (IsKeyPressed(KEY_T)) gameUI->toggleTeamStats();

    // Sélection de joueur avec clic
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
        Vector2 tileCoords = gameMap->getTileCoords(mousePos);

        for (auto& player : players) {
            float distance = sqrtf(powf(player.getPosition().x - tileCoords.x, 2) + 
                                 powf(player.getPosition().y - tileCoords.y, 2));
            if (distance < 1.0f) {
                gameUI->setSelectedPlayer(&player);
                break;
            }
        }
    }

    gameUI->handleInput();
}

void Game::update() {
    float deltaTime = GetFrameTime();

    for (auto& player : players) {
        player.update(deltaTime);
    }
}

void Game::render() {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    BeginMode2D(camera);

    // Dessiner la carte
    gameMap->draw(camera);

    // Dessiner les ressources
    for (const auto& resource : resources) {
        Vector2 worldPos = gameMap->getWorldPosition((int)resource.getPosition().x, (int)resource.getPosition().y);
        resource.draw(worldPos, gameMap->getTileSize());
    }

    // Dessiner les joueurs
    for (const auto& player : players) {
        Vector2 worldPos = gameMap->getWorldPosition((int)player.getPosition().x, (int)player.getPosition().y);
        player.draw(worldPos, gameMap->getTileSize());
    }

    EndMode2D();

    // Dessiner l'interface utilisateur
    gameUI->draw(players);

    // Afficher les FPS
    DrawText(TextFormat("FPS: %i", GetFPS()), 10, 10, 20, LIME);
    DrawText("Controls: WASD/Arrows - Move Camera, Mouse Wheel - Zoom, I - Player Info, T - Team Stats", 10, screenHeight - 30, 16, WHITE);

    EndDrawing();
}

void Game::run() {
    running = true;

    while (!WindowShouldClose() && running) {
        handleInput();
        update();
        render();
    }

    shutdown();
}

void Game::shutdown() {
    running = false;
    CloseWindow();
}