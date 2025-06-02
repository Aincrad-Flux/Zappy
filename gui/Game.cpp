/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** Game.cpp
*/

#include "Game.hpp"
#include <iostream>
#include <random>
#include <cmath>
#include <raymath.h>

Game::Game(int width, int height) : screenWidth(width), screenHeight(height), running(false), lastClickPosition({0, -1, 0}), selectedPlayerId(-1), debugMode(true)
{
    InitWindow(1400, 900, "Zappy GUI 3D - Raylib");
    screenWidth = 1400;
    screenHeight = 900;
    SetTargetFPS(60);

    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);

    gameMap = std::make_unique<Map>(20, 15, 32);
    gameUI = std::make_unique<UI>(screenWidth, screenHeight);

    float mapWidth = gameMap->getWidth() * gameMap->getTileSize();
    float mapHeight = gameMap->getHeight() * gameMap->getTileSize();

    camera.position = Vector3{ mapWidth / 2.0f, mapHeight * 1.2f, mapHeight * 0.8f };
    camera.target = Vector3{ mapWidth / 2.0f, 0.0f, mapHeight / 2.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Initialiser les données de test
    initializeMockData();

    // Centrer correctement la caméra pour voir toute la carte
    centerCamera();
}

Game::~Game() {}

void Game::initializeMockData()
{
    gameUI->addTeam("Team Alpha");
    gameUI->addTeam("Team Beta");
    gameUI->addTeam("Team Gamma");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> posDist(0, std::min(gameMap->getWidth(), gameMap->getHeight()) - 1);

    Color teamColors[] = {RED, BLUE, GREEN, YELLOW, PURPLE, ORANGE};
    std::string teamNames[] = {"Alpha", "Beta", "Gamma"};

    for (int i = 0; i < 6; i++) {
        Vector3 pos = {(float)posDist(gen), 0.0f, (float)posDist(gen)};
        players.emplace_back(i, teamNames[i % 3], pos, teamColors[i % 6]);
        gameMap->setTilePlayer((int)pos.x, (int)pos.z, 1);
    }

    std::uniform_int_distribution<> resourceDist(0, 6);
    for (int i = 0; i < 30; i++) {
        Vector3 pos = {(float)posDist(gen), 0.0f, (float)posDist(gen)};
        ResourceType type = (ResourceType)resourceDist(gen);
        resources.emplace_back(type, pos);
        gameMap->setTileResource((int)pos.x, (int)pos.z, (int)type);
    }
}

void Game::handleInput()
{
    const float cameraSpeed = 0.5f;
    const float rotationSpeed = 0.03f;
    const float verticalSpeed = 0.5f;

    // Gestion des clics de souris pour la sélection des joueurs
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        Ray mouseRay = GetMouseRay(mousePos, camera);

        if (debugMode) {
            printf("\n--- Mouse Click Debug Info ---\n");
            printf("Mouse position: (%.2f, %.2f)\n", mousePos.x, mousePos.y);
            printf("Ray origin: (%.2f, %.2f, %.2f)\n", mouseRay.position.x, mouseRay.position.y, mouseRay.position.z);
            printf("Ray direction: (%.2f, %.2f, %.2f)\n", mouseRay.direction.x, mouseRay.direction.y, mouseRay.direction.z);
        }

        // Vérifier si un joueur a été cliqué
        int previousSelectedId = selectedPlayerId;
        int clickedPlayerId = checkPlayerClick(mouseRay);

        if (clickedPlayerId >= 0) {
            selectedPlayerId = clickedPlayerId;
            if (debugMode) {
                printf("Player selected! ID: %d\n", selectedPlayerId);
            }
        } else {
            if (debugMode && previousSelectedId >= 0) {
                printf("No player hit. Previous selection maintained: %d\n", selectedPlayerId);
            } else {
                if (debugMode) {
                    printf("No player hit. Ray missed all cylindrical hitboxes.\n");
                }
            }

            // Si aucun joueur n'a été touché, calculer l'intersection avec le sol
            float t = -mouseRay.position.y / mouseRay.direction.y;
            if (t > 0) {
                Vector3 hitPoint = {
                    mouseRay.position.x + mouseRay.direction.x * t,
                    0,
                    mouseRay.position.z + mouseRay.direction.z * t
                };
                lastClickPosition = hitPoint;

                if (debugMode) {
                    printf("Click hit ground at: (%.2f, %.2f, %.2f)\n",
                           hitPoint.x, hitPoint.y, hitPoint.z);
                }
            }
        }
    }

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        Vector3 dir;
        dir.x = camera.position.x - camera.target.x;
        dir.y = camera.position.y - camera.target.y;
        dir.z = camera.position.z - camera.target.z;

        float distance = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        float len = distance;
        if (len > 0) {
            dir.x /= len;
            dir.y /= len;
            dir.z /= len;
        }

        float cosA = cosf(rotationSpeed);
        float sinA = sinf(rotationSpeed);
        float newX = dir.x * cosA - dir.z * sinA;
        float newZ = dir.x * sinA + dir.z * cosA;

        dir.x = newX;
        dir.z = newZ;

        camera.position.x = camera.target.x + dir.x * distance;
        camera.position.y = camera.target.y + dir.y * distance;
        camera.position.z = camera.target.z + dir.z * distance;
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        Vector3 dir;
        dir.x = camera.position.x - camera.target.x;
        dir.y = camera.position.y - camera.target.y;
        dir.z = camera.position.z - camera.target.z;

        float distance = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        float len = distance;
        if (len > 0) {
            dir.x /= len;
            dir.y /= len;
            dir.z /= len;
        }

        float cosA = cosf(-rotationSpeed);
        float sinA = sinf(-rotationSpeed);
        float newX = dir.x * cosA - dir.z * sinA;
        float newZ = dir.x * sinA + dir.z * cosA;

        dir.x = newX;
        dir.z = newZ;

        camera.position.x = camera.target.x + dir.x * distance;
        camera.position.y = camera.target.y + dir.y * distance;
        camera.position.z = camera.target.z + dir.z * distance;
    }

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        Vector3 dir;
        dir.x = camera.position.x - camera.target.x;
        dir.y = camera.position.y - camera.target.y;
        dir.z = camera.position.z - camera.target.z;

        float distance = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        float azimuth = atan2f(dir.z, dir.x);
        float elevation = atan2f(dir.y, sqrtf(dir.x*dir.x + dir.z*dir.z));
        elevation += rotationSpeed;

        if (elevation > 1.5f) elevation = 1.5f;

        camera.position.x = camera.target.x + distance * cosf(elevation) * cosf(azimuth);
        camera.position.y = camera.target.y + distance * sinf(elevation);
        camera.position.z = camera.target.z + distance * cosf(elevation) * sinf(azimuth);
    }

    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        Vector3 dir;
        dir.x = camera.position.x - camera.target.x;
        dir.y = camera.position.y - camera.target.y;
        dir.z = camera.position.z - camera.target.z;

        float distance = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        float azimuth = atan2f(dir.z, dir.x);
        float elevation = atan2f(dir.y, sqrtf(dir.x*dir.x + dir.z*dir.z));
        elevation -= rotationSpeed;

        if (elevation < 0.1f) elevation = 0.1f;
        camera.position.x = camera.target.x + distance * cosf(elevation) * cosf(azimuth);
        camera.position.y = camera.target.y + distance * sinf(elevation);
        camera.position.z = camera.target.z + distance * cosf(elevation) * sinf(azimuth);
    }

    if (IsKeyDown(KEY_Q)) {
        Vector3 dir;
        dir.x = camera.position.x - camera.target.x;
        dir.y = camera.position.y - camera.target.y;
        dir.z = camera.position.z - camera.target.z;

        float distance = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        float len = distance;
        if (len > 0) {
            dir.x /= len;
            dir.y /= len;
            dir.z /= len;
        }

        float cosA = cosf(-rotationSpeed);
        float sinA = sinf(-rotationSpeed);
        float newX = dir.x * cosA - dir.z * sinA;
        float newZ = dir.x * sinA + dir.z * cosA;

        dir.x = newX;
        dir.z = newZ;

        camera.position.x = camera.target.x + dir.x * distance;
        camera.position.y = camera.target.y + dir.y * distance;
        camera.position.z = camera.target.z + dir.z * distance;
    }

    if (IsKeyDown(KEY_E)) {
        Vector3 dir;
        dir.x = camera.position.x - camera.target.x;
        dir.y = camera.position.y - camera.target.y;
        dir.z = camera.position.z - camera.target.z;

        float distance = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        float len = distance;
        if (len > 0) {
            dir.x /= len;
            dir.y /= len;
            dir.z /= len;
        }

        float cosA = cosf(rotationSpeed);
        float sinA = sinf(rotationSpeed);
        float newX = dir.x * cosA - dir.z * sinA;
        float newZ = dir.x * sinA + dir.z * cosA;

        dir.x = newX;
        dir.z = newZ;

        camera.position.x = camera.target.x + dir.x * distance;
        camera.position.y = camera.target.y + dir.y * distance;
        camera.position.z = camera.target.z + dir.z * distance;
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Vector3 dir;
        dir.x = camera.position.x - camera.target.x;
        dir.y = camera.position.y - camera.target.y;
        dir.z = camera.position.z - camera.target.z;

        float distance = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z) - wheel * 2.0f;

        if (distance < 10.0f) distance = 10.0f;
        if (distance > 100.0f) distance = 100.0f;

        float len = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        if (len > 0) {
            dir.x /= len;
            dir.y /= len;
            dir.z /= len;
        }

        camera.position.x = camera.target.x + dir.x * distance;
        camera.position.y = camera.target.y + dir.y * distance;
        camera.position.z = camera.target.z + dir.z * distance;
    }

    if (IsKeyPressed(KEY_I)) gameUI->togglePlayerInfo();
    if (IsKeyPressed(KEY_T)) gameUI->toggleTeamStats();
    if (IsKeyPressed(KEY_M)) gameUI->toggleMenu();
    if (IsKeyPressed(KEY_H)) gameUI->toggleHelp();
    if (IsKeyPressed(KEY_F1)) debugMode = !debugMode;  // Basculer le mode debug avec F1
    if (IsKeyPressed(KEY_ESCAPE)) selectedPlayerId = -1;  // Désélectionner le joueur avec Echap
    if (IsKeyPressed(KEY_SPACE)) {
        float mapWidth = gameMap->getWidth() * gameMap->getTileSize();
        float mapHeight = gameMap->getHeight() * gameMap->getTileSize();
        camera.position = Vector3{ mapWidth / 2.0f, mapHeight * 1.2f, mapHeight * 0.8f };
        camera.target = Vector3{ mapWidth / 2.0f, 0.0f, mapHeight / 2.0f };
    }

    static Vector2 prevMousePos = { 0, 0 };
    const float mouseSensitivity = 0.02f;

    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            prevMousePos = mousePos;
        } else {
            float dx = mousePos.x - prevMousePos.x;
            float dy = mousePos.y - prevMousePos.y;

            Vector3 forward = Vector3Subtract(camera.target, camera.position);
            forward = Vector3Normalize(forward);

            Vector3 right = Vector3CrossProduct(forward, camera.up);
            right = Vector3Normalize(right);

            Vector3 moveRight = Vector3Scale(right, -dx * mouseSensitivity);
            Vector3 moveUp = Vector3Scale(camera.up, dy * mouseSensitivity);

            camera.target = Vector3Add(camera.target, moveRight);
            camera.target = Vector3Add(camera.target, moveUp);
            camera.position = Vector3Add(camera.position, moveRight);
            camera.position = Vector3Add(camera.position, moveUp);

            prevMousePos = mousePos;
        }
    }
}

void Game::update()
{
    float deltaTime = GetFrameTime();

    for (auto& player : players) {
        player.update(deltaTime);
    }
}

void Game::render()
{
    int newWidth = GetScreenWidth();
    int newHeight = GetScreenHeight();

    if (abs(newWidth - screenWidth) > 10 || abs(newHeight - screenHeight) > 10) {
        screenWidth = newWidth;
        screenHeight = newHeight;
        camera.fovy = 45.0f;
    }

    BeginDrawing();
    ClearBackground(DARKGRAY);
    BeginMode3D(camera);
    gameMap->draw();

    float mapWidth = gameMap->getWidth() * gameMap->getTileSize();
    float mapHeight = gameMap->getHeight() * gameMap->getTileSize();
    int slices = 20;
    float spacing = 1.0f;

    for (int i = 0; i <= slices; i++) {
        DrawLine3D({i * spacing, 0, 0}, {i * spacing, 0, slices * spacing}, LIGHTGRAY);
        DrawLine3D({0, 0, i * spacing}, {slices * spacing, 0, i * spacing}, LIGHTGRAY);
    }

    DrawLine3D({0, 0, 0}, {10, 0, 0}, RED);   // X axis
    DrawLine3D({0, 0, 0}, {0, 10, 0}, GREEN); // Y axis
    DrawLine3D({0, 0, 0}, {0, 0, 10}, BLUE);  // Z axis

    for (const auto& resource : resources) {
        Vector3 worldPos = gameMap->getWorldPosition((int)resource.getPosition().x, (int)resource.getPosition().z);
        resource.draw(worldPos, gameMap->getTileSize());
    }

    // Dessiner un indicateur pour le dernier clic
    if (lastClickPosition.y >= -0.5f) {
        // Pulse effect for the click indicator
        float pulseSize = 0.3f + sinf(GetTime() * 5.0f) * 0.1f;
        DrawSphere(lastClickPosition, pulseSize, RED);

        // Draw connecting lines to make it more visible
        DrawLine3D(lastClickPosition,
                  Vector3{lastClickPosition.x, lastClickPosition.y + 1.0f, lastClickPosition.z},
                  RED);
    }

    for (const auto& player : players) {
        Vector3 worldPos = gameMap->getWorldPosition((int)player.getPosition().x, (int)player.getPosition().z);
        player.draw(worldPos, gameMap->getTileSize());

        Vector3 center = {
            worldPos.x + gameMap->getTileSize()/2.0f,
            0.5f,
            worldPos.z + gameMap->getTileSize()/2.0f
        };
        float radius = gameMap->getTileSize() * 0.4f; // Même rayon que pour la détection

        // Dessiner la hitbox en mode debug
        if (debugMode) {
            // Dessiner un cylindre wireframe pour représenter la hitbox
            float height = gameMap->getTileSize() * 0.6f;
            DrawCylinderWires(center, radius, radius, height, 16, RED);
            DrawCircle3D({center.x, center.y - height/2.0f, center.z}, radius, {1, 0, 0}, 90.0f, RED);
            DrawCircle3D({center.x, center.y + height/2.0f, center.z}, radius, {1, 0, 0}, 90.0f, RED);
        }

        // Dessiner un indicateur pour le joueur sélectionné
        if (selectedPlayerId == player.getId()) {
            Vector3 markerPos = {
                worldPos.x + gameMap->getTileSize()/2.0f,
                1.5f + sinf(GetTime() * 3.0f) * 0.2f,
                worldPos.z + gameMap->getTileSize()/2.0f
            };
            DrawSphere(markerPos, 0.4f, YELLOW);
        }
    }

    EndMode3D();

    // Afficher les informations du joueur sélectionné
    if (selectedPlayerId >= 0) {
        for (const auto& player : players) {
            if (player.getId() == selectedPlayerId) {
                // Fond semi-transparent pour le panneau d'information
                DrawRectangle(10, 60, 250, 210, ColorAlpha(DARKGRAY, 0.8f));
                DrawRectangleLines(10, 60, 250, 210, WHITE);

                // Titre du panneau
                DrawText("PLAYER INFORMATION", 20, 70, 18, YELLOW);
                DrawLine(20, 90, 240, 90, WHITE);

                // Informations du joueur
                DrawText(TextFormat("Player ID: %d", player.getId()), 20, 100, 16, WHITE);
                DrawText(TextFormat("Team: %s", player.getTeamName().c_str()), 20, 120, 16, WHITE);
                DrawText(TextFormat("Level: %d", player.getLevel()), 20, 140, 16, WHITE);

                // Inventaire du joueur
                DrawText("Inventory:", 20, 160, 16, WHITE);
                DrawText(TextFormat("Food: %d", player.getInventory().getFood()), 40, 180, 16, WHITE);
                DrawText(TextFormat("Linemate: %d", player.getInventory().getLinemate()), 40, 200, 16, WHITE);
                DrawText(TextFormat("Deraumere: %d", player.getInventory().getDeraumere()), 40, 220, 16, WHITE);
                DrawText(TextFormat("Sibur: %d", player.getInventory().getSibur()), 40, 240, 16, WHITE);

                // Position du joueur
                DrawText(TextFormat("Position: (%.1f, %.1f)", player.getPosition().x, player.getPosition().z), 20, 260, 16, WHITE);
                break;
            }
        }
    }

    gameUI->draw(players);
    DrawText(TextFormat("FPS: %i", GetFPS()), 10, 10, 20, LIME);

    // Afficher un message d'aide si aucun joueur n'est sélectionné
    if (selectedPlayerId < 0) {
        DrawText("Click on a player to see their information", screenWidth/2 - 180, 30, 20, YELLOW);
    }

    if (debugMode) {
        DrawText("DEBUG MODE ON (F1 to toggle)", 10, 35, 16, YELLOW);
        DrawText("Click position shown with red sphere", 10, 55, 16, YELLOW);
        DrawText("Player hitboxes shown in red", 10, 75, 16, YELLOW);

        if (selectedPlayerId >= 0) {
            DrawText(TextFormat("CLICKED ON PLAYER ID: %d", selectedPlayerId), 10, 95, 16, GREEN);
        } else {
            DrawText("NO PLAYER SELECTED - TRY CLICKING ON A PLAYER", 10, 95, 16, RED);
        }
    }

    DrawText("Press H for help.", 10, screenHeight - 30, 16, WHITE);
    EndDrawing();
}

void Game::run()
{
    running = true;

    while (!WindowShouldClose() && running) {
        handleInput();
        update();
        render();
    }

    shutdown();
}

void Game::shutdown()
{
    running = false;
    CloseWindow();
}

void Game::centerCamera()
{
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    float mapWidth = (float)gameMap->getWidth() * gameMap->getTileSize();
    float mapHeight = (float)gameMap->getHeight() * gameMap->getTileSize();

    camera.position = Vector3{ mapWidth / 2.0f, mapHeight * 1.2f, mapHeight * 0.8f };
    camera.target = Vector3{ mapWidth / 2.0f, 0.0f, mapHeight / 2.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

int Game::checkPlayerClick(Ray mouseRay)
{
    float closestHit = 1000.0f;
    int hitPlayerId = -1;

    for (const auto& player : players) {
        if (!player.getIsAlive()) continue;

        Vector3 worldPos = gameMap->getWorldPosition((int)player.getPosition().x, (int)player.getPosition().z);
        Vector3 center = {
            worldPos.x + gameMap->getTileSize()/2.0f,
            0.5f,
            worldPos.z + gameMap->getTileSize()/2.0f
        };

        float tileSize = gameMap->getTileSize();
        float radius = tileSize * 0.3f;  // Utiliser le même rayon que celui du cylindre dessiné
        float height = tileSize * 0.6f;  // Utiliser la même hauteur que celle du cylindre dessiné

        // Calcul d'intersection rayon-cylindre
        // Équation paramétrique d'un rayon: p(t) = ray.position + t * ray.direction
        // Pour un cylindre vertical: (x - center.x)² + (z - center.z)² = radius²

        // Nous ne considérons que les coordonnées x et z pour le cylindre
        Vector2 rayPos2D = {mouseRay.position.x, mouseRay.position.z};
        Vector2 rayDir2D = {mouseRay.direction.x, mouseRay.direction.z};
        Vector2 cylinderPos2D = {center.x, center.z};

        // Vecteur de rayPos2D à cylinderPos2D
        Vector2 v = {rayPos2D.x - cylinderPos2D.x, rayPos2D.y - cylinderPos2D.y};

        // Coefficients de l'équation quadratique pour l'intersection
        float a = rayDir2D.x * rayDir2D.x + rayDir2D.y * rayDir2D.y;
        float b = 2.0f * (v.x * rayDir2D.x + v.y * rayDir2D.y);
        float c = v.x * v.x + v.y * v.y - radius * radius;

        float discriminant = b * b - 4 * a * c;

        if (discriminant < 0) {
            // Pas d'intersection avec le cylindre infini
            continue;
        }

        float t1 = (-b - sqrt(discriminant)) / (2 * a);
        float t2 = (-b + sqrt(discriminant)) / (2 * a);

        // On prend la plus petite valeur positive de t
        float t;
        if (t1 > 0 && t2 > 0) {
            t = fmin(t1, t2);
        } else if (t1 > 0) {
            t = t1;
        } else if (t2 > 0) {
            t = t2;
        } else {
            // Les deux intersections sont derrière la caméra
            continue;
        }

        // Vérifier si l'intersection est à l'intérieur de la hauteur du cylindre
        Vector3 intersection = {
            mouseRay.position.x + t * mouseRay.direction.x,
            mouseRay.position.y + t * mouseRay.direction.y,
            mouseRay.position.z + t * mouseRay.direction.z
        };

        if (intersection.y < center.y - height/2 || intersection.y > center.y + height/2) {
            // L'intersection n'est pas dans la hauteur du cylindre
            continue;
        }

        // Si cette intersection est la plus proche jusqu'à présent
        if (t < closestHit) {
            closestHit = t;
            hitPlayerId = player.getId();

            // Actualiser le point de clic
            lastClickPosition = intersection;

            if (debugMode) {
                printf("Hit player %d at position (%.2f, %.2f, %.2f)\n",
                       player.getId(),
                       intersection.x, intersection.y, intersection.z);
            }
        }
    }

    return hitPlayerId;
}