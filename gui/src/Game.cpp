/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** Game.cpp
*/

#include "Game.hpp"
#include "Logger.hpp"
#include <iostream>
#include <random>
#include <cmath>
#include <raymath.h>

Game::Game(int width, int height, const std::string& hostname, int port, bool use2D)
    : screenWidth(width), screenHeight(height), running(false), lastClickPosition({0, -1, 0}),
      selectedPlayerId(-1), selectedTile({-1, -1}), debugMode(false), use2DMode(use2D), serverHostname(hostname), serverPort(port),
      serverConnected(false), timeUnit(100)
{
    for (int i = 0; i < 7; i++) {
        tileResources[i] = 0;
    }

    Logger::getInstance().init("zappy_gui.log");
    Logger::getInstance().info("Game initialized with resolution " + std::to_string(width) + "x" + std::to_string(height) +
                               (use2DMode ? " (2D mode)" : " (3D mode)"));

    InitWindow(1400, 900, use2DMode ? "Zappy GUI 2D - Raylib" : "Zappy GUI 3D - Raylib");
    screenWidth = 1400;
    screenHeight = 900;
    SetTargetFPS(60);
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);

    camera.position = Vector3{ 0.0f, 20.0f, 20.0f };
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    gameUI = std::make_unique<UI>(screenWidth, screenHeight);

    networkManager = std::make_unique<NetworkManager>();
    gameUI = std::make_unique<UI>(screenWidth, screenHeight);

    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    if (!hostname.empty() && port > 0) {
        serverConnected = initializeNetworkConnection(hostname, port);
        if (serverConnected) {
            setupNetworkCallbacks();

            Logger::getInstance().info("Connected to server, waiting for map information");
            std::cout << "Connected to server, waiting for map information..." << std::endl;
        }
    }

    if (!serverConnected) {
        gameMap = std::make_unique<Map>(20, 15, 32);

        float mapWidth = gameMap->getWidth() * gameMap->getTileSize();
        float mapHeight = gameMap->getHeight() * gameMap->getTileSize();

        camera.position = Vector3{ mapWidth / 2.0f, mapHeight * 1.2f, mapHeight * 0.8f };
        camera.target = Vector3{ mapWidth / 2.0f, 0.0f, mapHeight / 2.0f };

        initializeMockData();
        centerCamera();
    }
}

Game::~Game() {
    Logger::getInstance().info("Game shutting down");
}

void Game::initializeMockData()
{
    Logger::getInstance().info("Initializing mock game data");

    std::string teamNames[] = {"Team Alpha", "Team Beta", "Team Gamma"};

    for (const auto& teamName : teamNames) {
        Color teamColor = getTeamColor(teamName);
        gameUI->addTeam(teamName);
        gameUI->setTeamColor(teamName, teamColor);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> posDist(0, std::min(gameMap->getWidth(), gameMap->getHeight()) - 1);

    for (int i = 0; i < 6; i++) {
        Vector3 pos = {(float)posDist(gen), 0.0f, (float)posDist(gen)};
        std::string teamName = teamNames[i % 3];
        Color teamColor = getTeamColor(teamName);
        players.emplace_back(i, teamName, pos, teamColor);
        gameMap->setTilePlayer((int)pos.x, (int)pos.z, 1);
    }

    std::uniform_int_distribution<> resourceDist(0, 6);
    std::uniform_int_distribution<> countDist(1, 6);

    for (int i = 0; i < 15; i++) {
        Vector3 basePos = {(float)posDist(gen), 0.0f, (float)posDist(gen)};
        ResourceType type = (ResourceType)resourceDist(gen);
        int count = countDist(gen);
        for (int j = 0; j < count; j++) {
            Vector3 pos = {basePos.x + j*0.01f, 0.0f, basePos.z + j*0.01f};
            resources.emplace_back(type, pos);
            gameMap->setTileResource((int)pos.x, (int)pos.z, (int)type);
        }
    }
}

void Game::handleInput()
{
    const float rotationSpeed = 0.03f;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        int previousSelectedId = selectedPlayerId;
        int clickedPlayerId = -1;

        if (gameMap) {
            if (use2DMode) {
                clickedPlayerId = checkPlayerClick2D(mousePos);
            } else {
                Ray mouseRay = GetMouseRay(mousePos, camera);

                if (debugMode) {
                    printf("\n--- Mouse Click Debug Info ---\n");
                    printf("Mouse position: (%.2f, %.2f)\n", mousePos.x, mousePos.y);
                    printf("Ray origin: (%.2f, %.2f, %.2f)\n", mouseRay.position.x, mouseRay.position.y, mouseRay.position.z);
                    printf("Ray direction: (%.2f, %.2f, %.2f)\n", mouseRay.direction.x, mouseRay.direction.y, mouseRay.direction.z);
                }

                clickedPlayerId = checkPlayerClick(mouseRay);

                if (clickedPlayerId < 0) {
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

                        int tileX = static_cast<int>(hitPoint.x) / gameMap->getTileSize();
                        int tileZ = static_cast<int>(hitPoint.z) / gameMap->getTileSize();

                        if (tileX >= 0 && tileX < gameMap->getWidth() &&
                            tileZ >= 0 && tileZ < gameMap->getHeight()) {
                            selectedTile = {(float)tileX, (float)tileZ};
                            selectedPlayerId = -1;
                            if (networkManager) {
                                networkManager->requestTileContent(tileX, tileZ);
                                std::string logMsg = "Requesting tile content at (" +
                                    std::to_string(tileX) + "," + std::to_string(tileZ) + ")";
                                Logger::getInstance().info(logMsg);
                                std::cout << logMsg << std::endl;
                            }
                        }
                    }
                }
            }
        }

        if (clickedPlayerId >= 0) {
            selectedPlayerId = clickedPlayerId;
            selectedTile = {-1, -1};
            if (debugMode) {
                printf("Player selected! ID: %d\n", selectedPlayerId);
            }
        } else {
            if (debugMode && previousSelectedId >= 0) {
                printf("No player hit. Previous selection maintained: %d\n", selectedPlayerId);
            } else if (debugMode) {
                printf("No player hit.\n");
            }
        }
    }

    if (!use2DMode) {
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

            float currentDistance = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
            float zoomFactor = wheel > 0 ? 0.85f : 1.3f;
            float newDistance = currentDistance * zoomFactor;

            if (newDistance < 5.0f) newDistance = 5.0f;
            if (newDistance > 1500.0f) newDistance = 1500.0f;

            float len = currentDistance;
            if (len > 0) {
                dir.x /= len;
                dir.y /= len;
                dir.z /= len;
            }
            camera.position.x = camera.target.x + dir.x * newDistance;
            camera.position.y = camera.target.y + dir.y * newDistance;
            camera.position.z = camera.target.z + dir.z * newDistance;
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

        if (IsKeyPressed(KEY_SPACE) && gameMap) {
            float mapWidth = gameMap->getWidth() * gameMap->getTileSize();
            float mapHeight = gameMap->getHeight() * gameMap->getTileSize();
            camera.position = Vector3{ mapWidth / 2.0f, mapHeight * 1.2f, mapHeight * 0.8f };
            camera.target = Vector3{ mapWidth / 2.0f, 0.0f, mapHeight / 2.0f };
        }
    }

    if (IsKeyPressed(KEY_I)) gameUI->togglePlayerInfo();
    if (IsKeyPressed(KEY_T)) gameUI->toggleTeamStats();
    if (IsKeyPressed(KEY_C)) gameUI->toggleMenu();
    if (IsKeyPressed(KEY_H)) gameUI->toggleHelp();
    if (IsKeyPressed(KEY_F1)) debugMode = !debugMode;
    if (IsKeyPressed(KEY_F2)) {
        use2DMode = !use2DMode;
        gameUI->set3DMode(!use2DMode);
        SetWindowTitle(use2DMode ? "Zappy GUI 2D - Raylib" : "Zappy GUI 3D - Raylib");
        centerCamera();
    }
    if (IsKeyPressed(KEY_ESCAPE)) selectedPlayerId = -1;
}

void Game::update()
{
    float deltaTime = GetFrameTime();

    if (serverConnected) {
        updateNetwork();
    }
    if (gameMap) {
        for (auto& player : players) {
            player.update(deltaTime);
        }
    }
}

void Game::render3DElements()
{
    BeginMode3D(camera);

    if (gameMap) {
        gameMap->draw();

        int slices = 20;
        float spacing = 1.0f;

        for (int i = 0; i <= slices; i++) {
            DrawLine3D({i * spacing, 0, 0}, {i * spacing, 0, slices * spacing}, LIGHTGRAY);
            DrawLine3D({0, 0, i * spacing}, {slices * spacing, 0, i * spacing}, LIGHTGRAY);
        }
    }

    DrawLine3D({0, 0, 0}, {10, 0, 0}, RED);   // Axe X
    DrawLine3D({0, 0, 0}, {0, 10, 0}, GREEN); // Axe Y
    DrawLine3D({0, 0, 0}, {0, 0, 10}, BLUE);  // Axe Z

    if (gameMap) {
        for (const auto& resource : resources) {
            Vector3 worldPos = gameMap->getWorldPosition((int)resource.getPosition().x, (int)resource.getPosition().z);
            resource.draw(worldPos, gameMap->getTileSize());
        }
    }

    // Indicateur de clic
    if (lastClickPosition.y >= -0.5f) {
        float pulseSize = 0.5f + sinf(GetTime() * 5.0f) * 0.15f;
        DrawSphere(lastClickPosition, pulseSize, RED);
        DrawLine3D(lastClickPosition,
                  Vector3{lastClickPosition.x, lastClickPosition.y + 1.5f, lastClickPosition.z},
                  RED);
    }

    // Players and their hitboxes - only render if map exists
    if (gameMap) {
        for (const auto& player : players) {
            Vector3 worldPos = gameMap->getWorldPosition((int)player.getPosition().x, (int)player.getPosition().z);
            player.draw(worldPos, gameMap->getTileSize());

            Vector3 center = {
                worldPos.x + gameMap->getTileSize()/2.0f,
                0.5f,
                worldPos.z + gameMap->getTileSize()/2.0f
            };

            if (debugMode) {
                float height = gameMap->getTileSize() * 0.6f;
                float hitboxRadius = gameMap->getTileSize() * 0.5f;
                DrawCylinderWires(center, hitboxRadius, hitboxRadius, height, 16, RED);
                DrawCircle3D({center.x, center.y - height/2.0f, center.z}, hitboxRadius, {1, 0, 0}, 90.0f, RED);
                DrawCircle3D({center.x, center.y + height/2.0f, center.z}, hitboxRadius, {1, 0, 0}, 90.0f, RED);
            }
            if (selectedPlayerId == player.getId()) {
                Vector3 markerPos = {
                    worldPos.x + gameMap->getTileSize()/2.0f,
                    1.5f + sinf(GetTime() * 3.0f) * 0.2f,
                    worldPos.z + gameMap->getTileSize()/2.0f
                };
                DrawSphere(markerPos, 0.4f, YELLOW);
            }
        }
    }
    EndMode3D();
}

void Game::render2DElements()
{
    if (!gameMap) return;

    int tileSize = gameMap->getTileSize();
    int mapWidth = gameMap->getWidth();
    int mapHeight = gameMap->getHeight();

    float scaleX = (float)(screenWidth * 0.8f) / (mapWidth * tileSize);
    float scaleY = (float)(screenHeight * 0.8f) / (mapHeight * tileSize);
    float scale = std::min(scaleX, scaleY);
    int offsetX = (screenWidth - mapWidth * tileSize * scale) / 2;
    int offsetY = (screenHeight - mapHeight * tileSize * scale) / 2;

    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            int tileX = offsetX + x * tileSize * scale;
            int tileY = offsetY + y * tileSize * scale;
            int tileW = tileSize * scale;
            int tileH = tileSize * scale;

            DrawRectangle(tileX, tileY, tileW, tileH, DARKGREEN);
            DrawRectangleLines(tileX, tileY, tileW, tileH, LIGHTGRAY);

            if (debugMode) {
                DrawText(TextFormat("%d,%d", x, y), tileX + 2, tileY + 2, 8, WHITE);
            }
        }
    }

    // Draw resources
    for (const auto& resource : resources) {
        int x = (int)resource.getPosition().x;
        int y = (int)resource.getPosition().z;
        int tileX = offsetX + x * tileSize * scale;
        int tileY = offsetY + y * tileSize * scale;
        int tileW = tileSize * scale;
        int tileH = tileSize * scale;

        Color resColor = WHITE;
        switch (resource.getType()) {
            case ResourceType::FOOD:      resColor = BROWN; break;
            case ResourceType::LINEMATE:  resColor = LIGHTGRAY; break;
            case ResourceType::DERAUMERE: resColor = BLUE; break;
            case ResourceType::SIBUR:     resColor = YELLOW; break;
            case ResourceType::MENDIANE:  resColor = PURPLE; break;
            case ResourceType::PHIRAS:    resColor = ORANGE; break;
            case ResourceType::THYSTAME:  resColor = PINK; break;
        }

        Vector2 center = {tileX + tileW/2.0f, tileY + tileH/2.0f};
        float radius = tileW * 0.1f;

        switch (resource.getType()) {
            case ResourceType::FOOD: // Circle
                DrawCircleV(center, radius, resColor);
                break;
            case ResourceType::LINEMATE: // Triangle
                DrawTriangle(
                    {center.x, center.y - radius},
                    {center.x - radius, center.y + radius},
                    {center.x + radius, center.y + radius},
                    resColor
                );
                break;
            case ResourceType::DERAUMERE: // Square
                DrawRectangle(center.x - radius, center.y - radius, radius*2, radius*2, resColor);
                break;
            default:
                DrawCircleV(center, radius, resColor);
                break;
        }
    }

    // Draw players
    for (const auto& player : players) {
        if (!player.getIsAlive()) continue;

        int x = (int)player.getPosition().x;
        int y = (int)player.getPosition().z;
        int tileX = offsetX + x * tileSize * scale;
        int tileY = offsetY + y * tileSize * scale;
        int tileW = tileSize * scale;
        int tileH = tileSize * scale;

        Color playerColor = player.getTeamColor();
        Vector2 center = {tileX + tileW/2.0f, tileY + tileH/2.0f};
        float radius = tileW * 0.35f;

        if (player.getIsIncanting()) {
            float pulse = sinf(GetTime() * 5.0f) * 0.3f + 0.7f;
            playerColor = ColorAlpha(playerColor, pulse);
            DrawCircleV(center, radius * 1.2f, ColorAlpha(WHITE, 0.3f));
        }

        DrawCircleV(center, radius, playerColor);

        Vector2 dirOffset = {0, 0};
        switch (player.getDirection()) {
            case PlayerDirection::NORTH: dirOffset = {0, -radius * 0.8f}; break;
            case PlayerDirection::EAST:  dirOffset = {radius * 0.8f, 0}; break;
            case PlayerDirection::SOUTH: dirOffset = {0, radius * 0.8f}; break;
            case PlayerDirection::WEST:  dirOffset = {-radius * 0.8f, 0}; break;
        }

        Vector2 frontPos = {center.x + dirOffset.x, center.y + dirOffset.y};
        DrawCircleV(frontPos, radius * 0.3f, WHITE);

        DrawText(TextFormat("L%d", player.getLevel()), tileX + tileW/2 - 10, tileY - 15, 14, WHITE);

        if (selectedPlayerId == player.getId()) {
            float pulseSize = 1.0f + sinf(GetTime() * 3.0f) * 0.1f;
            DrawCircleLines(center.x, center.y, radius * pulseSize, YELLOW);
            DrawCircleLines(center.x, center.y, radius * pulseSize * 1.1f, YELLOW);
        }
        float lifePercent = player.getLifeTime() / 1260.0f;
        Color lifeColor = (lifePercent > 0.5f) ? GREEN : (lifePercent > 0.25f) ? YELLOW : RED;
        DrawRectangle(tileX, tileY + tileH + 2, tileW * lifePercent, 4, lifeColor);
    }

    // Draw click indicator
    if (lastClickPosition.y >= -0.5f) {
        int clickX = offsetX + (int)lastClickPosition.x * tileSize * scale;
        int clickY = offsetY + (int)lastClickPosition.z * tileSize * scale;
        int tileW = tileSize * scale;
        int tileH = tileSize * scale;
        float pulseSize = 0.5f + sinf(GetTime() * 5.0f) * 0.15f;

        DrawCircleLinesV({(float)clickX + tileW/2, (float)clickY + tileH/2},
                        tileW * pulseSize, RED);
    }
}

void Game::renderDebugInfo()
{
    if (!debugMode) {
        return;
    }

    int debugPanelWidth = 300;
    int debugPanelHeight = 100;
    int debugPanelX = screenWidth - debugPanelWidth - 10;
    int debugPanelY = screenHeight - debugPanelHeight - 10;


    DrawRectangle(debugPanelX, debugPanelY, debugPanelWidth, debugPanelHeight, ColorAlpha(BLACK, 0.7f));
    DrawRectangleLines(debugPanelX, debugPanelY, debugPanelWidth, debugPanelHeight, RED);
    DrawText("DEBUG MODE (F1 to toggle)", debugPanelX + 10, debugPanelY + 10, 16, RED);
    DrawText("Click sphere & cylinder hitboxes shown in red", debugPanelX + 10, debugPanelY + 30, 14, YELLOW);

    if (selectedPlayerId >= 0) {
        DrawText(TextFormat("Selected player ID: %d", selectedPlayerId), debugPanelX + 10, debugPanelY + 50, 14, GREEN);

        for (const auto& player : players) {
            if (player.getId() == selectedPlayerId) {
                Vector3 pos = player.getPosition();
                DrawText(TextFormat("Player position: (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z),
                         debugPanelX + 10, debugPanelY + 70, 14, GREEN);
                break;
            }
        }
    } else {
        DrawText("No player selected - Click on a player", debugPanelX + 10, debugPanelY + 50, 14, YELLOW);
    }

    if (lastClickPosition.y >= -0.5f) {
        int yPos = selectedPlayerId >= 0 ? 90 : 70;
        DrawText(TextFormat("Last click: (%.1f, %.1f, %.1f)",
                 lastClickPosition.x, lastClickPosition.y, lastClickPosition.z),
                 debugPanelX + 10, debugPanelY + yPos, 14, WHITE);
    }
}

void Game::renderUIElements()
{
    if (selectedPlayerId >= 0) {
        for (const auto& player : players) {
            if (player.getId() == selectedPlayerId) {
                gameUI->setSelectedPlayer(const_cast<Player*>(&player));
                break;
            }
        }
    } else {
        gameUI->setSelectedPlayer(nullptr);
    }

    if (selectedTile.x >= 0 && selectedTile.y >= 0) {
        gameUI->setSelectedTile(selectedTile, tileResources);
    }

    gameUI->draw(players);
    DrawText(TextFormat("FPS: %i", GetFPS()), 400, 10, 20, LIME);

    if (!gameMap && serverConnected) {
        DrawText("Connected to server... Waiting for map data",
                screenWidth/2 - 220, screenHeight/2 - 20, 20, YELLOW);
    } else if (selectedPlayerId < 0 && !debugMode && gameMap) {
        DrawText("Click on a player to see their information", screenWidth/2 - 180, 30, 20, ColorAlpha(YELLOW, 0.7f));
    }

    if (!serverConnected) {
        const char* warningMsg = "NON CONNECTÉ AU SERVEUR";
        int fontSize = 40;
        int textWidth = MeasureText(warningMsg, fontSize);
        DrawRectangle(screenWidth/2 - textWidth/2 - 20,
                      screenHeight/2 - fontSize/2 - 20,
                      textWidth + 40,
                      fontSize + 40,
                      ColorAlpha(BLACK, 0.7f));
        DrawRectangleLines(screenWidth/2 - textWidth/2 - 20,
                          screenHeight/2 - fontSize/2 - 20,
                          textWidth + 40,
                          fontSize + 40,
                          RED);
        DrawText(warningMsg, screenWidth/2 - textWidth/2, screenHeight/2 - fontSize/2, fontSize, RED);
    }

    DrawText("Press H for help.", 10, screenHeight - 30, 16, WHITE);
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
    if (use2DMode) {
        render2DElements();
    } else {
        render3DElements();
    }
    renderDebugInfo();
    renderUIElements();
    EndDrawing();
}

void Game::run()
{
    Logger::getInstance().info("Game starting main loop");
    running = true;
    gameUI->set3DMode(!use2DMode);

    while (!WindowShouldClose() && running) {
        handleInput();
        update();
        render();
    }

    Logger::getInstance().info("Game exiting main loop");
    shutdown();
}

void Game::shutdown()
{
    Logger::getInstance().info("Game shutting down properly");
    running = false;
    CloseWindow();
}

void Game::centerCamera()
{
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    if (gameMap) {
        float mapWidth = (float)gameMap->getWidth() * gameMap->getTileSize();
        float mapHeight = (float)gameMap->getHeight() * gameMap->getTileSize();

    if (use2DMode) {
        camera.position = Vector3{ mapWidth / 2.0f, mapHeight * 3.0f, mapHeight / 2.0f };
        camera.target = Vector3{ mapWidth / 2.0f, 0.0f, mapHeight / 2.0f };
        camera.up = Vector3{ 0.0f, 0.0f, -1.0f };
        camera.fovy = 45.0f;
        camera.projection = CAMERA_ORTHOGRAPHIC;
    } else {
        camera.position = Vector3{ mapWidth / 2.0f, mapHeight * 1.2f, mapHeight * 0.8f };
        camera.target = Vector3{ mapWidth / 2.0f, 0.0f, mapHeight / 2.0f };
        camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;
    }
    }
}

bool Game::checkRayCylinderIntersection(const Ray& ray, const Vector3& center, float radius, float height, float& t, Vector3& hitPoint)
{
    Vector2 rayPos2D = {ray.position.x, ray.position.z};
    Vector2 rayDir2D = {ray.direction.x, ray.direction.z};
    Vector2 cylinderPos2D = {center.x, center.z};

    Vector2 v = {rayPos2D.x - cylinderPos2D.x, rayPos2D.y - cylinderPos2D.y};

    float a = rayDir2D.x * rayDir2D.x + rayDir2D.y * rayDir2D.y;

    if (fabs(a) < 0.00001f) {
        return false;
    }

    float b = 2.0f * (v.x * rayDir2D.x + v.y * rayDir2D.y);
    float c = v.x * v.x + v.y * v.y - radius * radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return false;
    }

    float sqrtDiscriminant = sqrtf(discriminant);
    float t1 = (-b - sqrtDiscriminant) / (2 * a);
    float t2 = (-b + sqrtDiscriminant) / (2 * a);

    if (t1 > 0 && t2 > 0) {
        t = fmin(t1, t2);
    } else if (t1 > 0) {
        t = t1;
    } else if (t2 > 0) {
        t = t2;
    } else {
        return false;
    }

    hitPoint = {
        ray.position.x + t * ray.direction.x,
        ray.position.y + t * ray.direction.y,
        ray.position.z + t * ray.direction.z
    };

    return (hitPoint.y >= center.y - height/2 && hitPoint.y <= center.y + height/2);
}

int Game::checkPlayerClick(Ray mouseRay)
{
    float closestHit = 1000.0f;
    int hitPlayerId = -1;

    if (!gameMap) {
        return -1;
    }

    for (const auto& player : players) {
        if (!player.getIsAlive()) {
            continue;
        }

        Vector3 worldPos = gameMap->getWorldPosition((int)player.getPosition().x, (int)player.getPosition().z);
        Vector3 center = {
            worldPos.x + gameMap->getTileSize()/2.0f,
            0.5f,
            worldPos.z + gameMap->getTileSize()/2.0f
        };

        float tileSize = gameMap->getTileSize();
        float radius = tileSize * 0.5f;
        float height = tileSize * 0.8f;
        float sphereRadius = radius * 1.2f;
        RayCollision sphereCollision = GetRayCollisionSphere(mouseRay, center, sphereRadius);

        if (!sphereCollision.hit) {
            continue;
        }

        float t;
        Vector3 intersection;
        bool hitCylinder = checkRayCylinderIntersection(mouseRay, center, radius, height, t, intersection);

        if (!hitCylinder) {
            intersection = sphereCollision.point;
            t = sphereCollision.distance;
        }

        if (t < closestHit) {
            closestHit = t;
            hitPlayerId = player.getId();
            lastClickPosition = intersection;

            if (debugMode) {
                printf("Hit player %d at position (%.2f, %.2f, %.2f)\n",
                       player.getId(), intersection.x, intersection.y, intersection.z);
            }
        }
    }

    if (hitPlayerId >= 0 && debugMode) {
        printf("✓ PLAYER %d SUCCESSFULLY SELECTED! Hit distance: %.2f\n", hitPlayerId, closestHit);
    }

    return hitPlayerId;
}

int Game::checkPlayerClick2D(Vector2 mousePos)
{
    if (!gameMap) return -1;

    int tileSize = gameMap->getTileSize();
    int mapWidth = gameMap->getWidth();
    int mapHeight = gameMap->getHeight();

    float scaleX = (float)(screenWidth * 0.8f) / (mapWidth * tileSize);
    float scaleY = (float)(screenHeight * 0.8f) / (mapHeight * tileSize);
    float scale = std::min(scaleX, scaleY);
    int offsetX = (screenWidth - mapWidth * tileSize * scale) / 2;
    int offsetY = (screenHeight - mapHeight * tileSize * scale) / 2;

    for (const auto& player : players) {
        if (!player.getIsAlive()) continue;

        int x = (int)player.getPosition().x;
        int y = (int)player.getPosition().z;
        int tileX = offsetX + x * tileSize * scale;
        int tileY = offsetY + y * tileSize * scale;
        int tileW = tileSize * scale;
        int tileH = tileSize * scale;

        Vector2 center = {tileX + tileW/2.0f, tileY + tileH/2.0f};
        float radius = tileW * 0.35f;

        float dx = mousePos.x - center.x;
        float dy = mousePos.y - center.y;
        float distSq = dx*dx + dy*dy;

        if (distSq <= radius*radius) {
            lastClickPosition = {(float)x, 0.0f, (float)y};

            if (debugMode) {
                printf("Hit player %d at tile position (%d, %d)\n", player.getId(), x, y);
            }

            return player.getId();
        }
    }

    int mapX = (mousePos.x - offsetX) / (tileSize * scale);
    int mapY = (mousePos.y - offsetY) / (tileSize * scale);

    if (mapX >= 0 && mapX < mapWidth && mapY >= 0 && mapY < mapHeight) {
        lastClickPosition = {(float)mapX, 0.0f, (float)mapY};

        selectedTile = {(float)mapX, (float)mapY};
        selectedPlayerId = -1;
        if (networkManager) {
            networkManager->requestTileContent(mapX, mapY);
            std::string logMsg = "Requesting tile content at (" +
                std::to_string(mapX) + "," + std::to_string(mapY) + ")";
            Logger::getInstance().info(logMsg);
            std::cout << logMsg << std::endl;
        }

        if (debugMode) {
            printf("Click on map at tile position (%d, %d)\n", mapX, mapY);
        }
    }

    return -1;
}

bool Game::initializeNetworkConnection(const std::string& hostname, int port)
{
    if (!networkManager) {
        networkManager = std::make_unique<NetworkManager>();
    }

    Logger::getInstance().info("Attempting to connect to server at " + hostname + ":" + std::to_string(port));
    bool result = networkManager->connect(hostname, port);
    if (!result) {
        Logger::getInstance().error("Failed to connect to server at " + hostname + ":" + std::to_string(port));
        std::cerr << "Failed to connect to server at " << hostname << ":" << port << std::endl;
    } else {
        Logger::getInstance().info("Successfully connected to server at " + hostname + ":" + std::to_string(port));
        std::cout << "Successfully connected to server at " << hostname << ":" << port << std::endl;
    }

    return result;
}

void Game::setupNetworkCallbacks()
{
    if (!networkManager) {
        Logger::getInstance().warning("Cannot setup network callbacks: NetworkManager is null");
        return;
    }

    Logger::getInstance().info("Setting up network callbacks");

    // Map size message (msz X Y\n)
    networkManager->registerCallback("msz", [this](const std::vector<std::string>& args) {
        if (args.size() >= 2) {
            int width = std::stoi(args[0]);
            int height = std::stoi(args[1]);

            std::string logMsg = "Received map size: " + std::to_string(width) + "x" + std::to_string(height);
            Logger::getInstance().info(logMsg);
            std::cout << logMsg << std::endl;

            resources.clear();
            players.clear();
            selectedPlayerId = -1;

            gameMap = std::make_unique<Map>(width, height, 32);

            float mapWidth = gameMap->getWidth() * gameMap->getTileSize();
            float mapHeight = gameMap->getHeight() * gameMap->getTileSize();

            camera.position = Vector3{ mapWidth / 2.0f, mapHeight * 1.2f, mapHeight * 0.8f };
            camera.target = Vector3{ mapWidth / 2.0f, 0.0f, mapHeight / 2.0f };

            centerCamera();
        }
    });

    // Tile content message (bct X Y q0 q1 q2 q3 q4 q5 q6\n)
    networkManager->registerCallback("bct", [this](const std::vector<std::string>& args) {
        if (args.size() >= 9) {
            int x = std::stoi(args[0]);
            int y = std::stoi(args[1]);
            int food = std::stoi(args[2]);      // q0
            int linemate = std::stoi(args[3]);  // q1
            int deraumere = std::stoi(args[4]); // q2
            int sibur = std::stoi(args[5]);     // q3
            int mendiane = std::stoi(args[6]);  // q4
            int phiras = std::stoi(args[7]);    // q5
            int thystame = std::stoi(args[8]);  // q6

            std::string logMsg = "Received tile content at (" + std::to_string(x) + "," + std::to_string(y) + ")";
            Logger::getInstance().debug(logMsg);

            if (selectedTile.x == x && selectedTile.y == y) {
                tileResources[0] = food;
                tileResources[1] = linemate;
                tileResources[2] = deraumere;
                tileResources[3] = sibur;
                tileResources[4] = mendiane;
                tileResources[5] = phiras;
                tileResources[6] = thystame;

                std::string selectedMsg = "Updated selected tile resources at (" + std::to_string(x) + "," + std::to_string(y) + ")";
                Logger::getInstance().info(selectedMsg);
            }

            if (food > 0) gameMap->setTileResource(x, y, 0, food);
            if (linemate > 0) gameMap->setTileResource(x, y, 1, linemate);
            if (deraumere > 0) gameMap->setTileResource(x, y, 2, deraumere);
            if (sibur > 0) gameMap->setTileResource(x, y, 3, sibur);
            if (mendiane > 0) gameMap->setTileResource(x, y, 4, mendiane);
            if (phiras > 0) gameMap->setTileResource(x, y, 5, phiras);
            if (thystame > 0) gameMap->setTileResource(x, y, 6, thystame);

            for (int i = 0; i < 7; ++i) {
                int count = 0;
                switch (i) {
                    case 0: count = food; break;
                    case 1: count = linemate; break;
                    case 2: count = deraumere; break;
                    case 3: count = sibur; break;
                    case 4: count = mendiane; break;
                    case 5: count = phiras; break;
                    case 6: count = thystame; break;
                }

                for (int j = 0; j < count; ++j) {
                    resources.emplace_back(static_cast<ResourceType>(i),
                                           Vector3{static_cast<float>(x) + j*0.01f, 0.0f, static_cast<float>(y) + j*0.01f});
                }
            }
        }
    });

    // Team names (tna N\n)
    networkManager->registerCallback("tna", [this](const std::vector<std::string>& args) {
        if (!args.empty()) {
            std::string teamName = args[0];
            std::string logMsg = "Received team name: " + teamName;
            Logger::getInstance().info(logMsg);
            std::cout << logMsg << std::endl;

            Color teamColor = getTeamColor(teamName);
            gameUI->addTeam(teamName);
            gameUI->setTeamColor(teamName, teamColor);
        }
    });

    // Player position (ppo #n X Y O\n)
    networkManager->registerCallback("ppo", [this](const std::vector<std::string>& args) {
        if (args.size() >= 4) {
            std::string playerIdStr = args[0];
            if (!playerIdStr.empty() && playerIdStr[0] == '#') {
                playerIdStr = playerIdStr.substr(1);
            }

            int playerId = std::stoi(playerIdStr);
            int x = std::stoi(args[1]);
            int y = std::stoi(args[2]);
            int orientation = std::stoi(args[3]); // 1(N), 2(E), 3(S), 4(W)

            std::string logMsg = "Received player position: #" + std::to_string(playerId) + " at (" +
                std::to_string(x) + "," + std::to_string(y) + ") facing " + std::to_string(orientation);
            Logger::getInstance().debug(logMsg);
            std::cout << logMsg << std::endl;

            bool found = false;
            for (auto& player : players) {
                if (player.getId() == playerId) {
                    found = true;
                    Vector3 oldPos = player.getPosition();
                    if (gameMap && oldPos.x != x && oldPos.z != y) {
                        gameMap->setTilePlayer(static_cast<int>(oldPos.x), static_cast<int>(oldPos.z), 0);
                    }

                    player.setPosition(Vector3{static_cast<float>(x), 0.0f, static_cast<float>(y)});

                    if (gameMap) {
                        gameMap->setTilePlayer(x, y, 1);
                    }

                    PlayerDirection dir;
                    switch(orientation) {
                        case 1: dir = PlayerDirection::NORTH; break;
                        case 2: dir = PlayerDirection::EAST; break;
                        case 3: dir = PlayerDirection::SOUTH; break;
                        case 4: dir = PlayerDirection::WEST; break;
                        default: dir = PlayerDirection::NORTH; break;
                    }
                    player.setDirection(dir);
                    break;
                }
            }

            if (!found) {
                Color defaultColor = getTeamColor("Unknown");
                players.emplace_back(playerId, "Unknown", Vector3{static_cast<float>(x), 0.0f, static_cast<float>(y)}, defaultColor);
                gameMap->setTilePlayer(x, y, 1);
            }
        }
    });

    // Player level (plv #n L\n)
    networkManager->registerCallback("plv", [this](const std::vector<std::string>& args) {
        if (args.size() >= 2) {
            std::string playerIdStr = args[0];
            if (!playerIdStr.empty() && playerIdStr[0] == '#') {
                playerIdStr = playerIdStr.substr(1);
            }

            int playerId = std::stoi(playerIdStr);
            int level = std::stoi(args[1]);

            std::string logMsg = "Received player level: #" + std::to_string(playerId) + " level " + std::to_string(level);
            Logger::getInstance().debug(logMsg);
            std::cout << logMsg << std::endl;

            for (auto& player : players) {
                if (player.getId() == playerId) {
                    player.setLevel(level);
                    break;
                }
            }
        }
    });

    // Player inventory (pin #n X Y q0 q1 q2 q3 q4 q5 q6\n)
    networkManager->registerCallback("pin", [this](const std::vector<std::string>& args) {
        if (args.size() >= 10) {
            std::string playerIdStr = args[0];
            if (!playerIdStr.empty() && playerIdStr[0] == '#') {
                playerIdStr = playerIdStr.substr(1);
            }

            int playerId = std::stoi(playerIdStr);
            int food = std::stoi(args[3]);
            int linemate = std::stoi(args[4]);
            int deraumere = std::stoi(args[5]);
            int sibur = std::stoi(args[6]);
            int mendiane = std::stoi(args[7]);
            int phiras = std::stoi(args[8]);
            int thystame = std::stoi(args[9]);

            std::string logMsg = "Received player #" + std::to_string(playerId) + " inventory";
            Logger::getInstance().debug(logMsg);
            std::cout << logMsg << std::endl;

            for (auto& player : players) {
                if (player.getId() == playerId) {
                    player.getInventory().setResource(ResourceType::FOOD, food);
                    player.getInventory().setResource(ResourceType::LINEMATE, linemate);
                    player.getInventory().setResource(ResourceType::DERAUMERE, deraumere);
                    player.getInventory().setResource(ResourceType::SIBUR, sibur);
                    player.getInventory().setResource(ResourceType::MENDIANE, mendiane);
                    player.getInventory().setResource(ResourceType::PHIRAS, phiras);
                    player.getInventory().setResource(ResourceType::THYSTAME, thystame);
                    break;
                }
            }
        }
    });

    // New player connection (pnw #n X Y O L N\n)
    networkManager->registerCallback("pnw", [this](const std::vector<std::string>& args) {
        if (args.size() >= 6) {
            std::string playerIdStr = args[0];
            if (playerIdStr[0] == '#') {
                playerIdStr = playerIdStr.substr(1);
            }

            int playerId = std::stoi(playerIdStr);
            int x = std::stoi(args[1]);
            int y = std::stoi(args[2]);
            int level = std::stoi(args[4]);
            std::string teamName = args[5];

            std::string logMsg = "New player #" + std::to_string(playerId) + " from team " + teamName + " at level " + std::to_string(level);
            Logger::getInstance().info(logMsg);
            std::cout << logMsg << std::endl;

            bool found = false;
            for (auto& player : players) {
                if (player.getId() == playerId) {
                    found = true;
                    player.setTeam(teamName);
                    player.setLevel(level);
                    player.setColor(getTeamColor(teamName));
                    break;
                }
            }

            if (!found) {
                Color teamColor = getTeamColor(teamName);
                players.emplace_back(playerId, teamName, Vector3{static_cast<float>(x), 0.0f, static_cast<float>(y)}, teamColor);
                players.back().setLevel(level);
                gameMap->setTilePlayer(x, y, 1);
            }
        }
    });

    // Player death (pdi #n\n)
    networkManager->registerCallback("pdi", [this](const std::vector<std::string>& args) {
        if (!args.empty()) {
            std::string playerIdStr = args[0];
            if (!playerIdStr.empty() && playerIdStr[0] == '#') {
                playerIdStr = playerIdStr.substr(1);
            }

            int playerId = std::stoi(playerIdStr);
            std::string logMsg = "Player #" + std::to_string(playerId) + " died";
            Logger::getInstance().info(logMsg);
            std::cout << logMsg << std::endl;

            for (auto& player : players) {
                if (player.getId() == playerId) {
                    player.setIsAlive(false);
                    Vector3 pos = player.getPosition();
                    gameMap->setTilePlayer(static_cast<int>(pos.x), static_cast<int>(pos.z), 0);
                    break;
                }
            }
        }
    });

    // End of game (seg N\n)
    networkManager->registerCallback("seg", [this](const std::vector<std::string>& args) {
        if (!args.empty()) {
            std::string winningTeam = args[0];
            std::string logMsg = "Game over! Team " + winningTeam + " wins!";
            Logger::getInstance().info(logMsg);
            std::cout << logMsg << std::endl;
            gameUI->showGameOverMessage("Team " + winningTeam + " wins!");
        } else {
            std::string logMsg = "Game over! No winner declared.";
            Logger::getInstance().info(logMsg);
            std::cout << logMsg << std::endl;
            gameUI->showGameOverMessage("Game Over!");
        }
    });

    // Time unit request (sgt T\n)
    networkManager->registerCallback("sgt", [this](const std::vector<std::string>& args) {
        if (!args.empty()) {
            timeUnit = std::stoi(args[0]);
            std::string logMsg = "Server time unit: " + std::to_string(timeUnit);
            Logger::getInstance().info(logMsg);
            std::cout << logMsg << std::endl;
        }
    });

    // Message from server (smg M\n)
    networkManager->registerCallback("smg", [this](const std::vector<std::string>& args) {
        if (!args.empty()) {
            std::string message = args[0];
            for (size_t i = 1; i < args.size(); ++i) {
                message += " " + args[i];
            }
            std::string logMsg = "Server message: " + message;
            Logger::getInstance().info(logMsg);
            std::cout << logMsg << std::endl;
            gameUI->showServerMessage(message);
        }
    });

    // Egg laying (pfk #n\n)
    networkManager->registerCallback("pfk", [](const std::vector<std::string>& args) {
        if (!args.empty()) {
            std::string playerIdStr = args[0];
            if (!playerIdStr.empty() && playerIdStr[0] == '#') {
                playerIdStr = playerIdStr.substr(1);
            }

            int playerId = std::stoi(playerIdStr);
            std::string logMsg = "Player #" + std::to_string(playerId) + " is laying an egg";
            Logger::getInstance().info(logMsg);
            std::cout << logMsg << std::endl;
        }
    });

    // Egg laid (enw #e #n X Y\n)
    networkManager->registerCallback("enw", [](const std::vector<std::string>& args) {
        if (args.size() >= 4) {
            try {
                std::string eggIdStr = args[0];
                std::string playerIdStr = args[1];
                std::string xStr = args[2];
                std::string yStr = args[3];

                if (eggIdStr.length() > 0 && eggIdStr[0] == '#') {
                    eggIdStr = eggIdStr.substr(1);
                }
                if (playerIdStr.length() > 0 && playerIdStr[0] == '#') {
                    playerIdStr = playerIdStr.substr(1);
                }

                if (eggIdStr.empty() || playerIdStr.empty() || xStr.empty() || yStr.empty()) {
                    throw std::invalid_argument("Empty parameter");
                }

                int eggId = std::stoi(eggIdStr);
                int playerId = (playerIdStr == "-1") ? -1 : std::stoi(playerIdStr);
                int x = std::stoi(xStr);
                int y = std::stoi(yStr);

                std::string logMsg = "Egg #" + std::to_string(eggId);
                if (playerId == -1) {
                    logMsg += " spawned by server";
                } else {
                    logMsg += " laid by player #" + std::to_string(playerId);
                }
                logMsg += " at (" + std::to_string(x) + "," + std::to_string(y) + ")";
                Logger::getInstance().info(logMsg);
                std::cout << logMsg << std::endl;
            } catch (const std::exception& e) {
                std::string errorMsg = "Error processing egg data: " + std::string(e.what());
                Logger::getInstance().error(errorMsg);
                std::cerr << errorMsg << std::endl;
            }
        }
    });

    // Egg hatching (ebo #e\n)
    networkManager->registerCallback("ebo", [](const std::vector<std::string>& args) {
        if (!args.empty()) {
            try {
                std::string eggIdStr = args[0];
                if (!eggIdStr.empty() && eggIdStr[0] == '#') {
                    eggIdStr = eggIdStr.substr(1);
                }

                int eggId = std::stoi(eggIdStr);
                std::string logMsg = "Egg #" + std::to_string(eggId) + " has hatched";
                Logger::getInstance().info(logMsg);
                std::cout << logMsg << std::endl;
            } catch (const std::exception& e) {
                std::string errorMsg = "Error processing egg hatching data: " + std::string(e.what());
                Logger::getInstance().error(errorMsg);
                std::cerr << errorMsg << std::endl;
            }
        }
    });

    // Egg death (edi #e\n)
    networkManager->registerCallback("edi", [](const std::vector<std::string>& args) {
        if (!args.empty()) {
            try {
                std::string eggIdStr = args[0];
                if (!eggIdStr.empty() && eggIdStr[0] == '#') {
                    eggIdStr = eggIdStr.substr(1);
                }

                int eggId = std::stoi(eggIdStr);
                std::string logMsg = "Egg #" + std::to_string(eggId) + " has died";
                Logger::getInstance().info(logMsg);
                std::cout << logMsg << std::endl;
            } catch (const std::exception& e) {
                std::string errorMsg = "Error processing egg death data: " + std::string(e.what());
                Logger::getInstance().error(errorMsg);
                std::cerr << errorMsg << std::endl;
            }
        }
    });
}

void Game::updateNetwork()
{
    if (!networkManager || !networkManager->isConnected()) {
        serverConnected = false;
        return;
    }
    networkManager->update();
}

Color Game::getTeamColor(const std::string& teamName)
{
    auto it = teamColors.find(teamName);
    if (it != teamColors.end()) {
        return it->second;
    }

    int colorHash = 0;
    for (char c : teamName) {
        colorHash += static_cast<int>(c);
    }

    Color teamColor = {
        static_cast<unsigned char>((colorHash * 124) % 200 + 55),
        static_cast<unsigned char>((colorHash * 91) % 200 + 55),
        static_cast<unsigned char>((colorHash * 137) % 200 + 55),
        255
    };

    teamColors[teamName] = teamColor;
    return teamColor;
}