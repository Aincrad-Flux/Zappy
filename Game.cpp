#include "Game.hpp"
#include <iostream>
#include <random>
#include <cmath>
#include <raymath.h>

Game::Game(int width, int height) : screenWidth(width), screenHeight(height), running(false)
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

    initializeMockData();
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

    for (const auto& player : players) {
        Vector3 worldPos = gameMap->getWorldPosition((int)player.getPosition().x, (int)player.getPosition().z);
        player.draw(worldPos, gameMap->getTileSize());
    }

    EndMode3D();
    gameUI->draw(players);
    DrawText(TextFormat("FPS: %i", GetFPS()), 10, 10, 20, LIME);
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