// filepath: /Users/pandorian/Delivery/B-YEP-400-LIL-4-1-zappy-thibault.pouch/gui/src/UI.cpp
/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** UI.cpp
*/

#include "UI.hpp"
#include "Resource.hpp"
#include "ResourceInfo.hpp"
#include "Logger.hpp"
#include <algorithm>

UI::UI(int width, int height) : screenWidth(width), screenHeight(height),
                                selectedPlayer(nullptr), showPlayerInfo(true), showTeamStats(false),
                                showMenu(true), showHelp(false), messageDisplayTime(0)
{
    font = GetFontDefault();
    Logger::getInstance().info("UI initialized with resolution " + std::to_string(width) + "x" + std::to_string(height));
}

UI::~UI() {
    Logger::getInstance().info("UI destroyed");
}

void UI::draw(const std::vector<Player>& players)
{
    (void)players;

    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    if (!showHelp) {
        drawGameStats();
        if (showMenu) {
            drawMenu();
            drawResourceLegend();
        }
        if (showPlayerInfo) {
            drawPlayerInfo();
        }
        if (showTeamStats) {
            drawTeamStats();
        }

        if (!gameOverMessage.empty()) {
            int msgWidth = MeasureText(gameOverMessage.c_str(), 40);
            DrawRectangle((screenWidth - msgWidth) / 2 - 20, screenHeight / 2 - 30, msgWidth + 40, 60, ColorAlpha(BLACK, 0.8f));
            DrawRectangleLines((screenWidth - msgWidth) / 2 - 20, screenHeight / 2 - 30, msgWidth + 40, 60, WHITE);
            DrawText(gameOverMessage.c_str(), (screenWidth - msgWidth) / 2, screenHeight / 2 - 20, 40, RED);
        }

        if (!serverMessage.empty()) {
            messageDisplayTime += GetFrameTime();
            if (messageDisplayTime < 5.0f) { // Display for 5 seconds
                int msgWidth = MeasureText(serverMessage.c_str(), 20);
                DrawRectangle((screenWidth - msgWidth) / 2 - 10, 70, msgWidth + 20, 30, ColorAlpha(BLACK, 0.8f));
                DrawRectangleLines((screenWidth - msgWidth) / 2 - 10, 70, msgWidth + 20, 30, WHITE);
                DrawText(serverMessage.c_str(), (screenWidth - msgWidth) / 2, 75, 20, YELLOW);
            } else {
                serverMessage = "";
                messageDisplayTime = 0;
            }
        }
    } else {
        drawHelp();
    }
}

void UI::drawPlayerInfo()
{
    int panelWidth = 250;
    int panelHeight = 380;
    int panelX = 10;
    int panelY = 230;

    DrawRectangle(panelX, panelY, panelWidth, panelHeight, ColorAlpha(BLACK, 0.8f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, WHITE);

    DrawText("Player Information", panelX + 10, panelY + 10, 20, WHITE);

    if (selectedPlayer && selectedPlayer->getIsAlive()) {
        int yOffset = panelY + 40;
        int lineHeight = 20;

        DrawText(TextFormat("ID: %d", selectedPlayer->getId()), panelX + 10, yOffset, 16, WHITE);
        yOffset += lineHeight;

        DrawText(TextFormat("Team: %s", selectedPlayer->getTeamName().c_str()), panelX + 10, yOffset, 16, WHITE);
        yOffset += lineHeight;

        DrawText(TextFormat("Level: %d", selectedPlayer->getLevel()), panelX + 10, yOffset, 16, WHITE);
        yOffset += lineHeight;

        DrawText(TextFormat("Position: (%.0f, %.0f)", selectedPlayer->getPosition().x, selectedPlayer->getPosition().y),
                panelX + 10, yOffset, 16, WHITE);
        yOffset += lineHeight;

        DrawText(TextFormat("Direction: %s", getDirectionString(selectedPlayer->getDirection()).c_str()),
                panelX + 10, yOffset, 16, WHITE);
        yOffset += lineHeight;

        DrawText("Inventory:", panelX + 10, yOffset, 16, WHITE);
        yOffset += lineHeight;

        // Get inventory and display each resource
        const Inventory& inventory = selectedPlayer->getInventory();
        int xOffset = 0;
        int itemsPerRow = 2;
        int itemCount = 0;
        int itemSpacing = 120;

        // Food
        DrawText(TextFormat("Food: %d", inventory.getFood()),
                panelX + 20 + xOffset, yOffset, 14, WHITE);

        xOffset += itemSpacing;

        // Linemate
        DrawText(TextFormat("Linemate: %d", inventory.getLinemate()),
                panelX + 20 + xOffset, yOffset, 14, WHITE);

        xOffset = 0;
        yOffset += lineHeight;

        // Deraumere
        DrawText(TextFormat("Deraumere: %d", inventory.getDeraumere()),
                panelX + 20 + xOffset, yOffset, 14, WHITE);

        xOffset += itemSpacing;

        // Sibur
        DrawText(TextFormat("Sibur: %d", inventory.getSibur()),
                panelX + 20 + xOffset, yOffset, 14, WHITE);

        xOffset = 0;
        yOffset += lineHeight;

        // Mendiane
        DrawText(TextFormat("Mendiane: %d", inventory.getMendiane()),
                panelX + 20 + xOffset, yOffset, 14, WHITE);

        xOffset += itemSpacing;

        // Phiras
        DrawText(TextFormat("Phiras: %d", inventory.getPhiras()),
                panelX + 20 + xOffset, yOffset, 14, WHITE);

        xOffset = 0;
        yOffset += lineHeight;

        // Thystame
        DrawText(TextFormat("Thystame: %d", inventory.getThystame()),
                panelX + 20 + xOffset, yOffset, 14, WHITE);
    } else {
        DrawText("No player selected or player is dead", panelX + 10, panelY + 40, 16, GRAY);
    }
}

void UI::drawTeamStats()
{
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

    int panelWidth = 250;
    int panelHeight = 210;
    int panelX = actualScreenWidth - panelWidth - 10;
    int panelY = 10;

    DrawRectangle(panelX, panelY, panelWidth, panelHeight, ColorAlpha(BLACK, 0.8f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, WHITE);

    DrawText("Team Statistics", panelX + 10, panelY + 10, 20, WHITE);

    int yOffset = panelY + 40;
    int lineHeight = 25;

    // Display team names with their associated colors
    for (size_t i = 0; i < teams.size(); i++) {
        const std::string& teamName = teams[i];
        Color teamColor = WHITE;

        auto it = teamColors.find(teamName);
        if (it != teamColors.end()) {
            teamColor = it->second;
        }

        DrawText(TextFormat("%s Team", teamName.c_str()), panelX + 10, yOffset, 16, teamColor);
        yOffset += lineHeight;
    }
}

void UI::drawResourceLegend()
{
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

    int panelWidth = 200;
    int panelHeight = 210;
    int panelX = 10;
    int panelY = 10;

    DrawRectangle(panelX, panelY, panelWidth, panelHeight, ColorAlpha(BLACK, 0.8f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, WHITE);

    DrawText("Resource Legend", panelX + 10, panelY + 10, 20, WHITE);

    int yOffset = panelY + 40;
    int lineHeight = 25;    // Display each resource type with its color
    // Food (Red)
    DrawRectangle(panelX + 10, yOffset, 10, 10, RED);
    DrawText("Food", panelX + 30, yOffset - 2, 16, WHITE);
    yOffset += lineHeight;

    // Linemate (Blue)
    DrawRectangle(panelX + 10, yOffset, 10, 10, BLUE);
    DrawText("Linemate", panelX + 30, yOffset - 2, 16, WHITE);
    yOffset += lineHeight;

    // Deraumere (Green)
    DrawRectangle(panelX + 10, yOffset, 10, 10, GREEN);
    DrawText("Deraumere", panelX + 30, yOffset - 2, 16, WHITE);
    yOffset += lineHeight;

    // Sibur (Yellow)
    DrawRectangle(panelX + 10, yOffset, 10, 10, YELLOW);
    DrawText("Sibur", panelX + 30, yOffset - 2, 16, WHITE);
    yOffset += lineHeight;

    // Mendiane (Purple)
    DrawRectangle(panelX + 10, yOffset, 10, 10, PURPLE);
    DrawText("Mendiane", panelX + 30, yOffset - 2, 16, WHITE);
    yOffset += lineHeight;

    // Phiras (Orange)
    DrawRectangle(panelX + 10, yOffset, 10, 10, ORANGE);
    DrawText("Phiras", panelX + 30, yOffset - 2, 16, WHITE);
    yOffset += lineHeight;

    // Thystame (Pink)
    DrawRectangle(panelX + 10, yOffset, 10, 10, PINK);
    DrawText("Thystame", panelX + 30, yOffset - 2, 16, WHITE);
}

void UI::drawGameStats()
{
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

    int timeHeight = 30;
    int panelX = actualScreenWidth / 2 - 50;
    int panelY = 10;

    DrawRectangle(panelX - 10, panelY, 140, timeHeight, ColorAlpha(BLACK, 0.8f));
    DrawRectangleLines(panelX - 10, panelY, 140, timeHeight, WHITE);

    DrawText("Time Frequency:", panelX - 5, panelY + 7, 14, WHITE);
    DrawText("F1-F5", panelX + 95, panelY + 7, 14, YELLOW);
}

void UI::showServerMessage(const std::string& message)
{
    serverMessage = message;
    messageDisplayTime = 0;
    Logger::getInstance().info("Server message displayed: " + message);
}

void UI::drawMenu()
{
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

    int menuWidth = 240;
    int menuHeight = 210;
    int menuX = actualScreenWidth - menuWidth - 10;
    int menuY = actualScreenHeight - menuHeight - 10;

    DrawRectangle(menuX, menuY, menuWidth, menuHeight, ColorAlpha(BLACK, 0.8f));
    DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, WHITE);

    DrawText("Zappy GUI Controls", menuX + 10, menuY + 10, 20, WHITE);

    int yOffset = menuY + 40;
    int lineHeight = 25;

    DrawText("M - Toggle Menu & Legend", menuX + 20, yOffset, 16, WHITE);
    yOffset += lineHeight;

    DrawText("I - Toggle Player Info Panel", menuX + 20, yOffset, 16, WHITE);
    yOffset += lineHeight;

    DrawText("T - Toggle Team Statistics", menuX + 20, yOffset, 16, WHITE);
    yOffset += lineHeight;

    DrawText("H - Show Help Screen", menuX + 20, yOffset, 16, WHITE);
    yOffset += lineHeight;

    DrawText("WASD/Arrows - Move Camera", menuX + 20, yOffset, 16, WHITE);
    yOffset += lineHeight;

    DrawText("Mouse Wheel - Zoom In/Out", menuX + 20, yOffset, 16, WHITE);
    yOffset += lineHeight;

    DrawText("SPACE - Center Camera View", menuX + 20, yOffset, 16, WHITE);
    yOffset += lineHeight;

    DrawText("Press M to hide this menu", menuX + menuWidth - 150, menuY + menuHeight - 20, 14, YELLOW);
}

void UI::showGameOverMessage(const std::string& message)
{
    gameOverMessage = message;
    Logger::getInstance().info("Game over message displayed: " + message);
}

void UI::handleInput()
{
    if (IsKeyPressed(KEY_I)) {
        togglePlayerInfo();
    }
    if (IsKeyPressed(KEY_T)) {
        toggleTeamStats();
    }
    if (IsKeyPressed(KEY_M)) {
        toggleMenu();
    }
    if (IsKeyPressed(KEY_H)) {
        toggleHelp();
    }
    if (IsKeyPressed(KEY_ESCAPE) && showHelp) {
        showHelp = false;
    }
}

void UI::setSelectedPlayer(Player* player)
{
    selectedPlayer = player;
}

void UI::addTeam(const std::string& teamName)
{
    // Check if team already exists
    if (std::find(teams.begin(), teams.end(), teamName) == teams.end()) {
        teams.push_back(teamName);
        Logger::getInstance().info("Added team: " + teamName);
    }
}

void UI::togglePlayerInfo()
{
    showPlayerInfo = !showPlayerInfo;
}

void UI::toggleTeamStats()
{
    showTeamStats = !showTeamStats;
}

void UI::toggleMenu()
{
    showMenu = !showMenu;
}

void UI::toggleHelp()
{
    showHelp = !showHelp;
}

void UI::drawHelp()
{
    int helpWidth = 600;
    int helpHeight = 500;
    int helpX = (screenWidth - helpWidth) / 2;
    int helpY = (screenHeight - helpHeight) / 2;

    DrawRectangle(helpX, helpY, helpWidth, helpHeight, ColorAlpha(BLACK, 0.9f));
    DrawRectangleLines(helpX, helpY, helpWidth, helpHeight, WHITE);

    DrawText("Zappy GUI Help", helpX + helpWidth/2 - 80, helpY + 20, 24, WHITE);

    int yOffset = helpY + 70;
    int lineHeight = 24;
    int colWidth = 230;

    DrawText("Camera Controls:", helpX + 20, yOffset, 18, YELLOW);
    yOffset += 30;

    DrawText("WASD", helpX + 40, yOffset, 16, WHITE);
    DrawText("Move Camera", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight;

    DrawText("Arrow Keys", helpX + 40, yOffset, 16, WHITE);
    DrawText("Move Camera", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight;

    DrawText("Mouse Wheel", helpX + 40, yOffset, 16, WHITE);
    DrawText("Zoom In/Out", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight;

    DrawText("SPACE", helpX + 40, yOffset, 16, WHITE);
    DrawText("Reset Camera View", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight * 1.5f;

    DrawText("Time Controls:", helpX + 20, yOffset, 18, YELLOW);
    yOffset += 30;

    DrawText("F1-F5", helpX + 40, yOffset, 16, WHITE);
    DrawText("Change Time Frequency", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight * 1.5f;

    DrawText("Interface Controls:", helpX + 20, yOffset, 18, YELLOW);
    yOffset += 30;

    DrawText("M", helpX + 40, yOffset, 16, WHITE);
    DrawText("Toggle Menu", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight;

    DrawText("I", helpX + 40, yOffset, 16, WHITE);
    DrawText("Toggle Player Information", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight;

    DrawText("T", helpX + 40, yOffset, 16, WHITE);
    DrawText("Toggle Team Statistics", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight;

    DrawText("H", helpX + 40, yOffset, 16, WHITE);
    DrawText("Toggle This Help Screen", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight;

    DrawText("ESC", helpX + 40, yOffset, 16, WHITE);
    DrawText("Close Help Screen", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight;

    DrawText("Left Click", helpX + 40, yOffset, 16, WHITE);
    DrawText("Select Player", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight * 1.5f;

    DrawText("About:", helpX + 20, yOffset, 18, YELLOW);
    yOffset += 30;

    DrawText("Zappy GUI - A graphical visualization tool", helpX + 40, yOffset, 16, WHITE);
    yOffset += lineHeight;

    DrawText("Press H or ESC to close this window", helpX + helpWidth/2 - 140, helpY + helpHeight - 30, 16, WHITE);
}

std::string UI::getDirectionString(PlayerDirection direction) {
    switch (direction) {
        case PlayerDirection::NORTH: return "North";
        case PlayerDirection::EAST: return "East";
        case PlayerDirection::SOUTH: return "South";
        case PlayerDirection::WEST: return "West";
        default: return "Unknown";
    }
}

void UI::setTeamColor(const std::string& teamName, Color color) {
    teamColors[teamName] = color;
}