#include "Game.hpp"
#include <iostream>
#include <random>
#include <cmath>
#include <raymath.h>     // Pour Vector3Add, Vector3Subtract, etc.

Game::Game(int width, int height) : screenWidth(width), screenHeight(height), running(false) {
    // Démarrer avec une fenêtre encore plus grande pour s'assurer que tout est visible
    InitWindow(1400, 900, "Zappy GUI 3D - Raylib");
    screenWidth = 1400; // S'assurer que screenWidth est mis à jour
    screenHeight = 900; // S'assurer que screenHeight est mis à jour
    SetTargetFPS(60);

    // Permettre le redimensionnement de la fenêtre
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED); // Commencer en mode maximisé

    gameMap = std::make_unique<Map>(20, 15, 32);
    gameUI = std::make_unique<UI>(screenWidth, screenHeight);

    // Initialize 3D camera
    // Position de la caméra dans un angle pour voir le plateau de jeu
    float mapWidth = gameMap->getWidth() * gameMap->getTileSize();
    float mapHeight = gameMap->getHeight() * gameMap->getTileSize();

    camera.position = Vector3{ mapWidth / 2.0f, mapHeight * 1.2f, mapHeight * 0.8f };
    camera.target = Vector3{ mapWidth / 2.0f, 0.0f, mapHeight / 2.0f }; // Regarder vers le centre de la carte
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };                           // Axe Y est vers le haut
    camera.fovy = 45.0f;                                               // Champ de vision
    camera.projection = CAMERA_PERSPECTIVE;                             // Mode de projection

    // Initialiser les données de test
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
        // En 3D, on utilise Vector3, avec Y pour la hauteur (0 pour être sur le sol)
        Vector3 pos = {(float)posDist(gen), 0.0f, (float)posDist(gen)};
        players.emplace_back(i, teamNames[i % 3], pos, teamColors[i % 6]);

        // Mettre à jour la carte
        gameMap->setTilePlayer((int)pos.x, (int)pos.z, 1);
    }

    // Créer quelques ressources de test
    std::uniform_int_distribution<> resourceDist(0, 6);
    for (int i = 0; i < 30; i++) {
        Vector3 pos = {(float)posDist(gen), 0.0f, (float)posDist(gen)};
        ResourceType type = (ResourceType)resourceDist(gen);
        resources.emplace_back(type, pos);

        // Mettre à jour la carte
        gameMap->setTileResource((int)pos.x, (int)pos.z, (int)type);
    }
}

void Game::handleInput() {
    // Contrôles de la caméra 3D
    const float cameraSpeed = 0.5f;
    const float rotationSpeed = 0.03f;

    // Déplacement de la caméra
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) camera.position.x += cameraSpeed;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) camera.position.x -= cameraSpeed;

    // Avancer/reculer par rapport à l'axe Z
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) camera.position.z += cameraSpeed;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) camera.position.z -= cameraSpeed;

    // Monter/descendre la caméra
    if (IsKeyDown(KEY_PAGE_UP)) camera.position.y += cameraSpeed;
    if (IsKeyDown(KEY_PAGE_DOWN)) camera.position.y -= cameraSpeed;

    // Rotation de la caméra autour de la cible
    if (IsKeyDown(KEY_Q)) {
        // Faire tourner la caméra autour de l'axe Y
        Vector3 dir;
        dir.x = camera.position.x - camera.target.x;
        dir.y = camera.position.y - camera.target.y;
        dir.z = camera.position.z - camera.target.z;

        float distance = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);

        // Normaliser le vecteur direction
        float len = distance;
        if (len > 0) {
            dir.x /= len;
            dir.y /= len;
            dir.z /= len;
        }

        // Rotation dans le sens antihoraire
        float cosA = cosf(-rotationSpeed);
        float sinA = sinf(-rotationSpeed);
        float newX = dir.x * cosA - dir.z * sinA;
        float newZ = dir.x * sinA + dir.z * cosA;

        dir.x = newX;
        dir.z = newZ;

        // Appliquer la nouvelle position
        camera.position.x = camera.target.x + dir.x * distance;
        camera.position.y = camera.target.y + dir.y * distance;
        camera.position.z = camera.target.z + dir.z * distance;
    }

    if (IsKeyDown(KEY_E)) {
        // Rotation dans l'autre sens
        Vector3 dir;
        dir.x = camera.position.x - camera.target.x;
        dir.y = camera.position.y - camera.target.y;
        dir.z = camera.position.z - camera.target.z;

        float distance = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);

        // Normaliser le vecteur direction
        float len = distance;
        if (len > 0) {
            dir.x /= len;
            dir.y /= len;
            dir.z /= len;
        }

        // Rotation dans le sens horaire
        float cosA = cosf(rotationSpeed);
        float sinA = sinf(rotationSpeed);
        float newX = dir.x * cosA - dir.z * sinA;
        float newZ = dir.x * sinA + dir.z * cosA;

        dir.x = newX;
        dir.z = newZ;

        // Appliquer la nouvelle position
        camera.position.x = camera.target.x + dir.x * distance;
        camera.position.y = camera.target.y + dir.y * distance;
        camera.position.z = camera.target.z + dir.z * distance;
    }

    // Zoom (ajuster la distance par rapport à la cible)
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Vector3 dir;
        dir.x = camera.position.x - camera.target.x;
        dir.y = camera.position.y - camera.target.y;
        dir.z = camera.position.z - camera.target.z;

        float distance = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z) - wheel * 2.0f;

        // Limiter la distance
        if (distance < 10.0f) distance = 10.0f;
        if (distance > 100.0f) distance = 100.0f;

        // Normaliser le vecteur direction
        float len = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        if (len > 0) {
            dir.x /= len;
            dir.y /= len;
            dir.z /= len;
        }

        // Appliquer la nouvelle position
        camera.position.x = camera.target.x + dir.x * distance;
        camera.position.y = camera.target.y + dir.y * distance;
        camera.position.z = camera.target.z + dir.z * distance;
    }

    // Interface - Gérer les touches directement ici
    if (IsKeyPressed(KEY_I)) gameUI->togglePlayerInfo();
    if (IsKeyPressed(KEY_T)) gameUI->toggleTeamStats();
    if (IsKeyPressed(KEY_M)) gameUI->toggleMenu();
    if (IsKeyPressed(KEY_H)) gameUI->toggleHelp();
    if (IsKeyPressed(KEY_SPACE)) {
        // Réinitialiser la caméra à sa position par défaut
        float mapWidth = gameMap->getWidth() * gameMap->getTileSize();
        float mapHeight = gameMap->getHeight() * gameMap->getTileSize();
        camera.position = Vector3{ mapWidth / 2.0f, mapHeight * 1.2f, mapHeight * 0.8f };
        camera.target = Vector3{ mapWidth / 2.0f, 0.0f, mapHeight / 2.0f };
    }

    // Sélection de joueur avec clic - pour 3D, il faudrait utiliser un ray casting
    // Cette partie serait à implémenter avec GetCollisionRayGround ou une autre approche
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

    // Si la taille a changé significativement, ajuster la caméra
    if (abs(newWidth - screenWidth) > 10 || abs(newHeight - screenHeight) > 10) {
        screenWidth = newWidth;
        screenHeight = newHeight;
        // Ajuster le champ de vision en fonction des nouvelles dimensions
        camera.fovy = 45.0f;
    }

    BeginDrawing();
    ClearBackground(DARKGRAY);

    BeginMode3D(camera);

    // Dessiner la carte
    gameMap->draw();

    // Dessiner un quadrillage pour faciliter l'orientation
    float mapWidth = gameMap->getWidth() * gameMap->getTileSize();
    float mapHeight = gameMap->getHeight() * gameMap->getTileSize();

    // Dessiner notre propre grille au lieu d'utiliser DrawGrid
    int slices = 20;
    float spacing = 1.0f;

    // Dessiner les lignes de la grille
    for (int i = 0; i <= slices; i++) {
        DrawLine3D({i * spacing, 0, 0}, {i * spacing, 0, slices * spacing}, LIGHTGRAY);
        DrawLine3D({0, 0, i * spacing}, {slices * spacing, 0, i * spacing}, LIGHTGRAY);
    }

    // Dessiner les axes pour l'orientation
    DrawLine3D({0, 0, 0}, {10, 0, 0}, RED);   // X axis
    DrawLine3D({0, 0, 0}, {0, 10, 0}, GREEN); // Y axis
    DrawLine3D({0, 0, 0}, {0, 0, 10}, BLUE);  // Z axis

    // Dessiner les ressources
    for (const auto& resource : resources) {
        Vector3 worldPos = gameMap->getWorldPosition((int)resource.getPosition().x, (int)resource.getPosition().z);
        resource.draw(worldPos, gameMap->getTileSize());
    }

    // Dessiner les joueurs
    for (const auto& player : players) {
        Vector3 worldPos = gameMap->getWorldPosition((int)player.getPosition().x, (int)player.getPosition().z);
        player.draw(worldPos, gameMap->getTileSize());
    }

    EndMode3D();

    // Dessiner l'interface utilisateur
    gameUI->draw(players);

    // Afficher les FPS
    DrawText(TextFormat("FPS: %i", GetFPS()), 10, 10, 20, LIME);

    // Afficher une note sur les contrôles disponibles
    DrawText("Press H for help, M for menu, Q/E to rotate, W/A/S/D to move, SPACE to reset camera",
             10, screenHeight - 30, 16, WHITE);

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
    // Cette fonction a été modifiée pour la version 3D
    // Mettre à jour les dimensions de la fenêtre
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    // Calculer la taille totale de la carte en pixels
    float mapWidth = (float)gameMap->getWidth() * gameMap->getTileSize();
    float mapHeight = (float)gameMap->getHeight() * gameMap->getTileSize();

    // Réinitialiser la caméra 3D
    camera.position = Vector3{ mapWidth / 2.0f, mapHeight * 1.2f, mapHeight * 0.8f };
    camera.target = Vector3{ mapWidth / 2.0f, 0.0f, mapHeight / 2.0f };

    // Pour la 3D, on définit l'angle de vue (FOV)
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}