#include "UI.hpp"
#include "Resource.hpp"
#include <algorithm>

UI::UI(int width, int height) : screenWidth(width), screenHeight(height),
                                selectedPlayer(nullptr), showPlayerInfo(true), showTeamStats(false) {
    font = GetFontDefault();
}

UI::~UI() {
    // Rien à faire pour le font par défaut
}

void UI::draw(const std::vector<Player>& players) {
    (void)players;

    drawResourceLegend();
    drawGameStats();

    if (showPlayerInfo) {
        drawPlayerInfo();
    }

    if (showTeamStats) {
        drawTeamStats();
    }
}

void UI::drawPlayerInfo() {
    int panelWidth = 300;
    int panelHeight = 400;
    int panelX = screenWidth - panelWidth - 10;
    int panelY = 50;

    // Panel background
    DrawRectangle(panelX, panelY, panelWidth, panelHeight, ColorAlpha(BLACK, 0.8f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, WHITE);

    // Title
    DrawText("Player Information", panelX + 10, panelY + 10, 20, WHITE);

    if (selectedPlayer && selectedPlayer->getIsAlive()) {
        int yOffset = panelY + 40;
        int lineHeight = 20;

        // Basic info
        DrawText(TextFormat("ID: %d", selectedPlayer->getId()), panelX + 10, yOffset, 16, WHITE);
        yOffset += lineHeight;

        DrawText(TextFormat("Team: %s", selectedPlayer->getTeamName().c_str()), panelX + 10, yOffset, 16, WHITE);
        yOffset += lineHeight;

        DrawText(TextFormat("Level: %d", selectedPlayer->getLevel()), panelX + 10, yOffset, 16, WHITE);
        yOffset += lineHeight;

        DrawText(TextFormat("Position: (%.0f, %.0f)", selectedPlayer->getPosition().x, selectedPlayer->getPosition().y),
                panelX + 10, yOffset, 16, WHITE);
        yOffset += lineHeight;

        // Direction
        const char* dirNames[] = {"North", "East", "South", "West"};
        DrawText(TextFormat("Direction: %s", dirNames[(int)selectedPlayer->getDirection()]),
                panelX + 10, yOffset, 16, WHITE);
        yOffset += lineHeight;

        // Life time
        float lifePercent = selectedPlayer->getLifeTime() / 1260.0f * 100.0f;
        Color lifeColor = (lifePercent > 50.0f) ? GREEN : (lifePercent > 25.0f) ? YELLOW : RED;
        DrawText(TextFormat("Life: %.1f%% (%.0f units)", lifePercent, selectedPlayer->getLifeTime()),
                panelX + 10, yOffset, 16, lifeColor);
        yOffset += lineHeight + 10;

        // Status
        if (selectedPlayer->getIsIncanting()) {
            DrawText("STATUS: INCANTING", panelX + 10, yOffset, 16, YELLOW);
            yOffset += lineHeight;
        }

        yOffset += 10;

        // Inventory
        DrawText("Inventory:", panelX + 10, yOffset, 18, WHITE);
        yOffset += 25;

        Inventory inv = selectedPlayer->getInventory();
        DrawText(TextFormat("Food: %d", inv.food), panelX + 20, yOffset, 14, BROWN);
        yOffset += 18;
        DrawText(TextFormat("Linemate: %d", inv.linemate), panelX + 20, yOffset, 14, LIGHTGRAY);
        yOffset += 18;
        DrawText(TextFormat("Deraumere: %d", inv.deraumere), panelX + 20, yOffset, 14, BLUE);
        yOffset += 18;
        DrawText(TextFormat("Sibur: %d", inv.sibur), panelX + 20, yOffset, 14, YELLOW);
        yOffset += 18;
        DrawText(TextFormat("Mendiane: %d", inv.mendiane), panelX + 20, yOffset, 14, PURPLE);
        yOffset += 18;
        DrawText(TextFormat("Phiras: %d", inv.phiras), panelX + 20, yOffset, 14, ORANGE);
        yOffset += 18;
        DrawText(TextFormat("Thystame: %d", inv.thystame), panelX + 20, yOffset, 14, PINK);

    } else {
        DrawText("No player selected", panelX + 10, panelY + 50, 16, GRAY);
        DrawText("Click on a player to", panelX + 10, panelY + 70, 16, GRAY);
        DrawText("view their information", panelX + 10, panelY + 90, 16, GRAY);
    }
}

void UI::drawTeamStats() {
    int panelWidth = 250;
    int panelHeight = 300;
    int panelX = 10;
    int panelY = 50;

    // Panel background
    DrawRectangle(panelX, panelY, panelWidth, panelHeight, ColorAlpha(BLACK, 0.8f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, WHITE);

    // Title
    DrawText("Team Statistics", panelX + 10, panelY + 10, 20, WHITE);

    int yOffset = panelY + 40;
    int lineHeight = 25;

    Color teamColors[] = {RED, BLUE, GREEN, YELLOW, PURPLE, ORANGE};

    for (size_t i = 0; i < teams.size() && i < 6; i++) {
        DrawText(TextFormat("%s Team", teams[i].c_str()), panelX + 10, yOffset, 16, teamColors[i]);
        yOffset += lineHeight;

        // Ici on pourrait ajouter des statistiques par équipe
        // Nombre de joueurs, niveau moyen, etc.
    }
}

void UI::drawResourceLegend() {
    int legendX = 10;
    int legendY = screenHeight - 200;
    int legendWidth = 200;
    int legendHeight = 180;

    // Panel background
    DrawRectangle(legendX, legendY, legendWidth, legendHeight, ColorAlpha(BLACK, 0.8f));
    DrawRectangleLines(legendX, legendY, legendWidth, legendHeight, WHITE);

    // Title
    DrawText("Resource Legend", legendX + 10, legendY + 10, 16, WHITE);

    int yOffset = legendY + 35;
    int lineHeight = 20;

    // Liste des ressources avec leurs couleurs
    struct ResourceInfo {
        const char* name;
        Color color;
        const char* shape;
    };

    ResourceInfo resources[] = {
        {"Food", BROWN, "Circle"},
        {"Linemate", LIGHTGRAY, "Triangle"},
        {"Deraumere", BLUE, "Square"},
        {"Sibur", YELLOW, "Diamond"},
        {"Mendiane", PURPLE, "Hexagon"},
        {"Phiras", ORANGE, "Star"},
        {"Thystame", PINK, "Cross"}
    };

    for (int i = 0; i < 7; i++) {
        // Dessiner un petit échantillon de la ressource
        Vector2 center = {(float)(legendX + 20), (float)(yOffset + 8)};

        switch (i) {
            case 0: // Food
                DrawCircleV(center, 4, resources[i].color);
                break;
            case 1: // Linemate
                DrawTriangle({center.x, center.y - 4}, {center.x - 3, center.y + 2}, {center.x + 3, center.y + 2}, resources[i].color);
                break;
            case 2: // Deraumere
                DrawRectangle((int)(center.x - 3), (int)(center.y - 3), 6, 6, resources[i].color);
                break;
            default:
                DrawCircleV(center, 4, resources[i].color);
                break;
        }

        DrawText(resources[i].name, legendX + 40, yOffset, 14, WHITE);
        yOffset += lineHeight;
    }
}

void UI::drawGameStats() {
    int statsX = screenWidth - 200;
    int statsY = 10;

    DrawText("Zappy Game", statsX, statsY, 20, WHITE);
    DrawText(TextFormat("Time: %.1fs", GetTime()), statsX, statsY + 25, 16, WHITE);
}

void UI::handleInput() {
    // Gestion des inputs spécifiques à l'UI si nécessaire
}

void UI::setSelectedPlayer(Player* player) {
    selectedPlayer = player;
}

void UI::addTeam(const std::string& teamName) {
    if (std::find(teams.begin(), teams.end(), teamName) == teams.end()) {
        teams.push_back(teamName);
    }
}

void UI::togglePlayerInfo() {
    showPlayerInfo = !showPlayerInfo;
}

void UI::toggleTeamStats() {
    showTeamStats = !showTeamStats;
}