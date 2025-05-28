#include "Game.hpp"
#include <iostream>
#include <random>
#include <cmath>

Game::Game(int width, int height) : screenWidth(width), screenHeight(height), running(false) {
    // Démarrer avec une fenêtre encore plus grande pour s'assurer que tout est visible
    InitWindow(1400, 900, "Zappy GUI - Raylib");
    screenWidth = 1400; // S'assurer que screenWidth est mis à jour
    screenHeight = 900; // S'assurer que screenHeight est mis à jour
    SetTargetFPS(60);

    // Permettre le redimensionnement de la fenêtre
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED); // Commencer en mode maximisé

    gameMap = std::make_unique<Map>(20, 15, 32);
    gameUI = std::make_unique<UI>(screenWidth, screenHeight);

    // Initialize camera
    camera.target = { (float)gameMap->getWidth() * gameMap->getTileSize() / 2.0f,
                      (float)gameMap->getHeight() * gameMap->getTileSize() / 2.0f };
    camera.offset = { (float)screenWidth / 2.0f, (float)screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 0.5f; // Commencer avec un zoom plus éloigné pour voir plus de la carte

    // Initialiser les données de test
    initializeMockData();

    // Centrer correctement la caméra pour voir toute la carte
    centerCamera();
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
        if (camera.zoom < 0.4f) camera.zoom = 0.4f;  // Permettre un zoom plus large
        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
    }

    // Interface - Gérer les touches directement ici
    if (IsKeyPressed(KEY_I)) gameUI->togglePlayerInfo();
    if (IsKeyPressed(KEY_T)) gameUI->toggleTeamStats();
    if (IsKeyPressed(KEY_M)) gameUI->toggleMenu();
    if (IsKeyPressed(KEY_H)) gameUI->toggleHelp();
    if (IsKeyPressed(KEY_SPACE)) centerCamera(); // Recentrer la caméra avec la touche espace

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
}

void Game::update() {
    float deltaTime = GetFrameTime();

    for (auto& player : players) {
        player.update(deltaTime);
    }
}

void Game::render() {
    // Mettre à jour les dimensions de l'écran en cas de redimensionnement
    int newWidth = GetScreenWidth();
    int newHeight = GetScreenHeight();

    // Si la taille a changé significativement, recalculer la position de la caméra
    if (abs(newWidth - screenWidth) > 10 || abs(newHeight - screenHeight) > 10) {
        screenWidth = newWidth;
        screenHeight = newHeight;
        centerCamera(); // Recentrer la caméra quand la fenêtre change de taille
    }

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

    // Afficher une note sur les contrôles disponibles
    DrawText("Press H for help, M to toggle menu, SPACE to center view", 10, screenHeight - 30, 16, WHITE);

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

void Game::centerCamera() {
    // Mettre à jour les dimensions de la fenêtre
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    // Calculer la taille totale de la carte en pixels
    float mapWidth = (float)gameMap->getWidth() * gameMap->getTileSize();
    float mapHeight = (float)gameMap->getHeight() * gameMap->getTileSize();

    // Centrer la caméra sur la carte
    camera.target = {
        mapWidth / 2.0f,
        mapHeight / 2.0f
    };

    // Recalculer l'offset de la caméra
    camera.offset = {
        (float)screenWidth / 2.0f,
        (float)screenHeight / 2.0f
    };

    // Approche plus directe pour le zoom
    // Calculer la taille de la carte par rapport à la fenêtre
    float scaleX = (float)(screenWidth * 0.7f) / mapWidth;  // Utiliser seulement 70% de la largeur pour laisser de la place aux menus
    float scaleY = (float)(screenHeight * 0.8f) / mapHeight;  // Utiliser 80% de la hauteur

    // Utiliser le plus petit facteur pour s'assurer que toute la carte est visible avec une marge
    camera.zoom = std::min(scaleX, scaleY) * 0.9f;
}