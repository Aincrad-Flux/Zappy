/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** UI.cpp
*/

#include "UI.hpp"
#include "Resource.hpp"
#include "ResourceInfo.hpp"
#include <algorithm>

UI::UI(int width, int height) : screenWidth(width), screenHeight(height),
                                selectedPlayer(nullptr), showPlayerInfo(true), showTeamStats(false),
                                showMenu(true), showHelp(false), messageDisplayTime(0)
{
    font = GetFontDefault();
}



UI::~UI() {}

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
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

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

        const char* dirNames[] = {"North", "East", "South", "West"};
        DrawText(TextFormat("Direction: %s", dirNames[(int)selectedPlayer->getDirection()]),
                panelX + 10, yOffset, 16, WHITE);
        yOffset += lineHeight;

        float lifePercent = selectedPlayer->getLifeTime() / 1260.0f * 100.0f;
        Color lifeColor = (lifePercent > 50.0f) ? GREEN : (lifePercent > 25.0f) ? YELLOW : RED;
        DrawText(TextFormat("Life: %.1f%% (%.0f units)", lifePercent, selectedPlayer->getLifeTime()),
                panelX + 10, yOffset, 16, lifeColor);
        yOffset += lineHeight + 10;

        if (selectedPlayer->getIsIncanting()) {
            DrawText("STATUS: INCANTING", panelX + 10, yOffset, 16, YELLOW);
            yOffset += lineHeight;
        }

        yOffset += 10;

        DrawText("Inventory:", panelX + 10, yOffset, 18, WHITE);
        yOffset += 25;

        Inventory inv = selectedPlayer->getInventory();
        DrawText(TextFormat("Food: %d", inv.getFood()), panelX + 20, yOffset, 14, BROWN);
        yOffset += 18;
        DrawText(TextFormat("Linemate: %d", inv.getLinemate()), panelX + 20, yOffset, 14, LIGHTGRAY);
        yOffset += 18;
        DrawText(TextFormat("Deraumere: %d", inv.getDeraumere()), panelX + 20, yOffset, 14, BLUE);
        yOffset += 18;
        DrawText(TextFormat("Sibur: %d", inv.getSibur()), panelX + 20, yOffset, 14, YELLOW);
        yOffset += 18;
        DrawText(TextFormat("Mendiane: %d", inv.getMendiane()), panelX + 20, yOffset, 14, PURPLE);
        yOffset += 18;
        DrawText(TextFormat("Phiras: %d", inv.getPhiras()), panelX + 20, yOffset, 14, ORANGE);
        yOffset += 18;
        DrawText(TextFormat("Thystame: %d", inv.getThystame()), panelX + 20, yOffset, 14, PINK);

    } else {
        DrawText("No player selected", panelX + 10, panelY + 50, 16, GRAY);
        DrawText("Click on a player to", panelX + 10, panelY + 70, 16, GRAY);
        DrawText("view their information", panelX + 10, panelY + 90, 16, GRAY);
    }
}

void UI::drawTeamStats()
{
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

    int panelWidth = 250;
    int panelHeight = 300;
    int panelX = actualScreenWidth - panelWidth - 10;
    int panelY = 240;

    DrawRectangle(panelX, panelY, panelWidth, panelHeight, ColorAlpha(BLACK, 0.8f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, WHITE);

    DrawText("Team Statistics", panelX + 10, panelY + 10, 20, WHITE);

    int yOffset = panelY + 40;
    int lineHeight = 25;

    Color teamColors[] = {RED, BLUE, GREEN, YELLOW, PURPLE, ORANGE};

    for (size_t i = 0; i < teams.size() && i < 6; i++) {
        DrawText(TextFormat("%s Team", teams[i].c_str()), panelX + 10, yOffset, 16, teamColors[i]);
        yOffset += lineHeight;
    }
}

void UI::drawResourceLegend()
{
    int actualScreenHeight = GetScreenHeight();
    int actualScreenWidth = GetScreenWidth();

    int legendWidth = 180;
    int legendHeight = 210;
    int legendX = 10;
    int legendY = 10;

    DrawRectangle(legendX, legendY, legendWidth, legendHeight, ColorAlpha(BLACK, 0.8f));
    DrawRectangleLines(legendX, legendY, legendWidth, legendHeight, WHITE);

    DrawText("Resource Legend", legendX + 10, legendY + 10, 18, WHITE);

    int yOffset = legendY + 35;
    int lineHeight = 20;

    ResourceInfo resources[] = {
        ResourceInfo("Food", BROWN, "Circle"),
        ResourceInfo("Linemate", LIGHTGRAY, "Triangle"),
        ResourceInfo("Deraumere", BLUE, "Square"),
        ResourceInfo("Sibur", YELLOW, "Diamond"),
        ResourceInfo("Mendiane", PURPLE, "Hexagon"),
        ResourceInfo("Phiras", ORANGE, "Star"),
        ResourceInfo("Thystame", PINK, "Cross")
    };

    for (int i = 0; i < 7; i++) {
        Vector2 center = {(float)(legendX + 20), (float)(yOffset + 8)};

        switch (i) {
            case 0: // Food
                DrawCircleV(center, 4, resources[i].getColor());
                break;
            case 1: // Linemate
                DrawTriangle({center.x, center.y - 4}, {center.x - 3, center.y + 2}, {center.x + 3, center.y + 2}, resources[i].getColor());
                break;
            case 2: // Deraumere
                DrawRectangle((int)(center.x - 3), (int)(center.y - 3), 6, 6, resources[i].getColor());
                break;
            default:
                DrawCircleV(center, 4, resources[i].getColor());
                break;
        }

        DrawText(resources[i].getName(), legendX + 40, yOffset, 14, WHITE);
        yOffset += lineHeight;
    }
}

void UI::drawGameStats()
{
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

    int statsX = actualScreenWidth / 2 - 100;
    int statsY = 15;

    DrawText("Zappy Game", statsX, statsY, 24, WHITE);
    DrawText(TextFormat("Time: %.1fs", GetTime()), statsX, statsY + 30, 18, WHITE);
}

void UI::handleInput()
{
    if (showHelp && IsKeyPressed(KEY_ESCAPE)) {
        showHelp = false;
    }
}

void UI::setSelectedPlayer(Player* player)
{
    selectedPlayer = player;
}

void UI::addTeam(const std::string& teamName)
{
    if (std::find(teams.begin(), teams.end(), teamName) == teams.end()) {
        teams.push_back(teamName);
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

void UI::showGameOverMessage(const std::string& message)
{
    gameOverMessage = message;
}

void UI::showServerMessage(const std::string& message)
{
    serverMessage = message;
    messageDisplayTime = 0;
}

void UI::drawMenu()
{
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

    int menuWidth = 220;
    int menuHeight = 210;

    int menuX = actualScreenWidth - menuWidth - 10;
    int menuY = 10;

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

void UI::drawHelp()
{
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

    int helpWidth = 700;
    int helpHeight = 550;
    int helpX = (actualScreenWidth - helpWidth) / 2;
    int helpY = (actualScreenHeight - helpHeight) / 2;

    DrawRectangle(0, 0, actualScreenWidth, actualScreenHeight, ColorAlpha(BLACK, 0.7f));

    DrawRectangle(helpX, helpY, helpWidth, helpHeight, ColorAlpha(DARKGRAY, 0.9f));
    DrawRectangleLines(helpX, helpY, helpWidth, helpHeight, WHITE);

    DrawText("Zappy GUI - Help & Controls", helpX + 20, helpY + 20, 28, WHITE);

    int yOffset = helpY + 70;
    int lineHeight = 25;
    int colWidth = 280;

    DrawText("Camera Controls:", helpX + 20, yOffset, 18, YELLOW);
    yOffset += 30;

    DrawText("Left / Right Arrows (or A/D)", helpX + 40, yOffset, 16, WHITE);
    DrawText("Rotate Camera", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight;

    DrawText("Up / Down Arrows (or W/S)", helpX + 40, yOffset, 16, WHITE);
    DrawText("Rotate Camera Pitch (Up/Down)", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight;

    DrawText("Right Mouse Button + Drag", helpX + 40, yOffset, 16, WHITE);
    DrawText("Pan Camera View", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight;

    DrawText("Mouse Wheel", helpX + 40, yOffset, 16, WHITE);
    DrawText("Zoom In / Out", helpX + 40 + colWidth, yOffset, 16, GRAY);
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