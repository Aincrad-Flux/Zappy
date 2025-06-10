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

Game::Game(int width, int height, const std::string& hostname, int port)
    : screenWidth(width), screenHeight(height), running(false), lastClickPosition({0, -1, 0}),
      selectedPlayerId(-1), debugMode(false), serverHostname(hostname), serverPort(port),
      serverConnected(false), timeUnit(100) // serverPort is used in initializeNetworkConnection
{
    Logger::getInstance().init("zappy_gui.log");
    Logger::getInstance().info("Game initialized with resolution " + std::to_string(width) + "x" + std::to_string(height));
    InitWindow(1400, 900, "Zappy GUI 3D - Raylib");
    screenWidth = 1400;
    screenHeight = 900;
    SetTargetFPS(60);

    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);

    networkManager = std::make_unique<NetworkManager>();
    gameMap = std::make_unique<Map>(20, 15, 32);
    gameUI = std::make_unique<UI>(screenWidth, screenHeight);

    float mapWidth = gameMap->getWidth() * gameMap->getTileSize();
    float mapHeight = gameMap->getHeight() * gameMap->getTileSize();

    camera.position = Vector3{ mapWidth / 2.0f, mapHeight * 1.2f, mapHeight * 0.8f };
    camera.target = Vector3{ mapWidth / 2.0f, 0.0f, mapHeight / 2.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    if (!hostname.empty() && port > 0) {
        serverConnected = initializeNetworkConnection(hostname, port);
        if (serverConnected) {
            // Setup callbacks for network messages
            setupNetworkCallbacks();

            // Request initial data from server
            networkManager->getMapSize(width, height);
            networkManager->requestTeamNames();
            networkManager->requestTimeUnit();
        }
    }
    if (!serverConnected) {
        initializeMockData();
    }
    centerCamera();
}

Game::~Game() {
    Logger::getInstance().info("Game shutting down");
}

void Game::initializeMockData()
{
    Logger::getInstance().info("Initializing mock game data");
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
    const float rotationSpeed = 0.03f;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        Ray mouseRay = GetMouseRay(mousePos, camera);

        if (debugMode) {
            printf("\n--- Mouse Click Debug Info ---\n");
            printf("Mouse position: (%.2f, %.2f)\n", mousePos.x, mousePos.y);
            printf("Ray origin: (%.2f, %.2f, %.2f)\n", mouseRay.position.x, mouseRay.position.y, mouseRay.position.z);
            printf("Ray direction: (%.2f, %.2f, %.2f)\n", mouseRay.direction.x, mouseRay.direction.y, mouseRay.direction.z);
        }

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

    // Update network state and process messages if connected
    if (serverConnected) {
        updateNetwork();
    }

    for (auto& player : players) {
        player.update(deltaTime);
    }
}

void Game::render3DElements()
{
    BeginMode3D(camera);
    gameMap->draw();

    int slices = 20;
    float spacing = 1.0f;

    for (int i = 0; i <= slices; i++) {
        DrawLine3D({i * spacing, 0, 0}, {i * spacing, 0, slices * spacing}, LIGHTGRAY);
        DrawLine3D({0, 0, i * spacing}, {slices * spacing, 0, i * spacing}, LIGHTGRAY);
    }

    DrawLine3D({0, 0, 0}, {10, 0, 0}, RED);   // Axe X
    DrawLine3D({0, 0, 0}, {0, 10, 0}, GREEN); // Axe Y
    DrawLine3D({0, 0, 0}, {0, 0, 10}, BLUE);  // Axe Z

    for (const auto& resource : resources) {
        Vector3 worldPos = gameMap->getWorldPosition((int)resource.getPosition().x, (int)resource.getPosition().z);
        resource.draw(worldPos, gameMap->getTileSize());
    }

    // Indicateur de clic
    if (lastClickPosition.y >= -0.5f) {
        float pulseSize = 0.5f + sinf(GetTime() * 5.0f) * 0.15f;
        DrawSphere(lastClickPosition, pulseSize, RED);
        DrawLine3D(lastClickPosition,
                  Vector3{lastClickPosition.x, lastClickPosition.y + 1.5f, lastClickPosition.z},
                  RED);
    }

    // Players and their hitboxes
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
    EndMode3D();
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

    gameUI->draw(players);
    DrawText(TextFormat("FPS: %i", GetFPS()), 10, 10, 20, LIME);
    if (selectedPlayerId < 0 && !debugMode) {
        DrawText("Click on a player to see their information", screenWidth/2 - 180, 30, 20, ColorAlpha(YELLOW, 0.7f));
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
    render3DElements();
    renderDebugInfo();
    renderUIElements();
    EndDrawing();
}

void Game::run()
{
    Logger::getInstance().info("Game starting main loop");
    running = true;

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

    float mapWidth = (float)gameMap->getWidth() * gameMap->getTileSize();
    float mapHeight = (float)gameMap->getHeight() * gameMap->getTileSize();

    camera.position = Vector3{ mapWidth / 2.0f, mapHeight * 1.2f, mapHeight * 0.8f };
    camera.target = Vector3{ mapWidth / 2.0f, 0.0f, mapHeight / 2.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
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

            // Create new map with server-provided dimensions
            gameMap = std::make_unique<Map>(width, height, 32);
            centerCamera();

            // Request content for all tiles
            networkManager->requestMapContent();
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
            std::cout << logMsg << std::endl;

            if (food > 0) gameMap->setTileResource(x, y, 0, food);
            if (linemate > 0) gameMap->setTileResource(x, y, 1, linemate);
            if (deraumere > 0) gameMap->setTileResource(x, y, 2, deraumere);
            if (sibur > 0) gameMap->setTileResource(x, y, 3, sibur);
            if (mendiane > 0) gameMap->setTileResource(x, y, 4, mendiane);
            if (phiras > 0) gameMap->setTileResource(x, y, 5, phiras);
            if (thystame > 0) gameMap->setTileResource(x, y, 6, thystame);

            // Add resource objects for each resource type on the tile
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
                    resources.emplace_back(static_cast<ResourceType>(i), Vector3{static_cast<float>(x), 0.0f, static_cast<float>(y)});
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
            gameUI->addTeam(teamName);
        }
    });

    // Player position (ppo #n X Y O\n)
    networkManager->registerCallback("ppo", [this](const std::vector<std::string>& args) {
        if (args.size() >= 4) {
            int playerId = std::stoi(args[0]);
            int x = std::stoi(args[1]);
            int y = std::stoi(args[2]);
            int orientation = std::stoi(args[3]); // 1(N), 2(E), 3(S), 4(W)

            std::string logMsg = "Received player position: #" + std::to_string(playerId) + " at (" + 
                std::to_string(x) + "," + std::to_string(y) + ") facing " + std::to_string(orientation);
            Logger::getInstance().debug(logMsg);
            std::cout << logMsg << std::endl;

            // Find player or create if not exists
            bool found = false;
            for (auto& player : players) {
                if (player.getId() == playerId) {
                    found = true;
                    player.setPosition(Vector3{static_cast<float>(x), 0.0f, static_cast<float>(y)});

                    // Convert from server orientation (1-4) to our direction enum (0-3)
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
                // For new players, we don't know the team yet
                // We'll update this with the "pnw" message
                Color defaultColor = BLUE; // Temporary color until team is known
                players.emplace_back(playerId, "Unknown", Vector3{static_cast<float>(x), 0.0f, static_cast<float>(y)}, defaultColor);
                gameMap->setTilePlayer(x, y, 1);
            }
        }
    });

    // Player level (plv #n L\n)
    networkManager->registerCallback("plv", [this](const std::vector<std::string>& args) {
        if (args.size() >= 2) {
            int playerId = std::stoi(args[0]);
            int level = std::stoi(args[1]);

            std::string logMsg = "Received player level: #" + std::to_string(playerId) + " level " + std::to_string(level);
            Logger::getInstance().debug(logMsg);
            std::cout << logMsg << std::endl;

            // Find player and update level
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
            int playerId = std::stoi(args[0]);
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

            // Find player and update inventory
            for (auto& player : players) {
                if (player.getId() == playerId) {
                    // Update player's inventory with resources
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
            int playerId = std::stoi(args[0]);
            int x = std::stoi(args[1]);
            int y = std::stoi(args[2]);
            // orientation only used for debug output, can be kept
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
                    player.setColor(teamColor);
                    break;
                }
            }

            if (!found) {
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

                players.emplace_back(playerId, teamName, Vector3{static_cast<float>(x), 0.0f, static_cast<float>(y)}, teamColor);
                players.back().setLevel(level);
                gameMap->setTilePlayer(x, y, 1);
            }
        }
    });

    // Player death (pdi #n\n)
    networkManager->registerCallback("pdi", [this](const std::vector<std::string>& args) {
        if (!args.empty()) {
            int playerId = std::stoi(args[0]);
            std::string logMsg = "Player #" + std::to_string(playerId) + " died";
            Logger::getInstance().info(logMsg);
            std::cout << logMsg << std::endl;

            // Find player and mark as dead
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
            int playerId = std::stoi(args[0]);
            std::string logMsg = "Player #" + std::to_string(playerId) + " is laying an egg";
            Logger::getInstance().info(logMsg);
            std::cout << logMsg << std::endl;
        }
    });

    // Egg laid (enw #e #n X Y\n)
    networkManager->registerCallback("enw", [](const std::vector<std::string>& args) {
        if (args.size() >= 4) {
            int eggId = std::stoi(args[0]);
            int playerId = std::stoi(args[1]);
            int x = std::stoi(args[2]);
            int y = std::stoi(args[3]);

            std::string logMsg = "Egg #" + std::to_string(eggId) + " laid by player #" + 
                std::to_string(playerId) + " at (" + std::to_string(x) + "," + std::to_string(y) + ")";
            Logger::getInstance().info(logMsg);
            std::cout << logMsg << std::endl;
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