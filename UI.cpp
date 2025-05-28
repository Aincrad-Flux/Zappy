#include "UI.hpp"
#include "Resource.hpp"
#include <algorithm>

UI::UI(int width, int height) : screenWidth(width), screenHeight(height),
                                selectedPlayer(nullptr), showPlayerInfo(true), showTeamStats(false),
                                showMenu(true), showHelp(false) {
    font = GetFontDefault();
}



UI::~UI() {
    // Rien à faire pour le font par défaut
}

void UI::draw(const std::vector<Player>& players) {
    (void)players;

    // Récupérer les dimensions actuelles de l'écran à chaque frame
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    if (!showHelp) {
        // Les statistiques de jeu sont toujours affichées
        drawGameStats();

        // La légende des ressources et le menu sont affichés ensemble
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
    } else {
        // Help screen takes priority over everything else
        drawHelp();
    }
}

void UI::drawPlayerInfo() {
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

    int panelWidth = 250; // Réduire un peu la taille
    int panelHeight = 380;
    int panelX = 10; // Positionné à gauche
    int panelY = 230; // En dessous de la légende des ressources

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
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

    int panelWidth = 250;
    int panelHeight = 300;
    int panelX = actualScreenWidth - panelWidth - 10; // Droite
    int panelY = 240; // Sous le menu principal

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
    int actualScreenHeight = GetScreenHeight();
    int actualScreenWidth = GetScreenWidth();

    int legendWidth = 180;
    int legendHeight = 210;
    int legendX = 10; // Place la légende à gauche
    int legendY = 10; // En haut à gauche

    // Panel background
    DrawRectangle(legendX, legendY, legendWidth, legendHeight, ColorAlpha(BLACK, 0.8f));
    DrawRectangleLines(legendX, legendY, legendWidth, legendHeight, WHITE);

    // Title
    DrawText("Resource Legend", legendX + 10, legendY + 10, 18, WHITE);

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
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

    int statsX = actualScreenWidth / 2 - 100; // Centré en haut de l'écran
    int statsY = 15;

    DrawText("Zappy Game", statsX, statsY, 24, WHITE);
    DrawText(TextFormat("Time: %.1fs", GetTime()), statsX, statsY + 30, 18, WHITE);
}

void UI::handleInput() {
    // Fermer la fenêtre d'aide avec la touche ESCAPE
    if (showHelp && IsKeyPressed(KEY_ESCAPE)) {
        showHelp = false;
    }
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

void UI::toggleMenu() {
    showMenu = !showMenu;
}

void UI::toggleHelp() {
    showHelp = !showHelp;
}

void UI::drawMenu() {
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

    // Dimensions du menu adaptées à la taille de l'écran
    int menuWidth = 220;
    int menuHeight = 210;

    // Position du menu dans le coin supérieur droit
    int menuX = actualScreenWidth - menuWidth - 10;
    int menuY = 10;

    // Panel de fond
    DrawRectangle(menuX, menuY, menuWidth, menuHeight, ColorAlpha(BLACK, 0.8f));
    DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, WHITE);

    // Titre du menu
    DrawText("Zappy GUI Controls", menuX + 10, menuY + 10, 20, WHITE);

    // Options du menu
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

    // Information sur les commandes de base
    DrawText("WASD/Arrows - Move Camera", menuX + 20, yOffset, 16, WHITE);
    yOffset += lineHeight;

    DrawText("Mouse Wheel - Zoom In/Out", menuX + 20, yOffset, 16, WHITE);
    yOffset += lineHeight;

    DrawText("SPACE - Center Camera View", menuX + 20, yOffset, 16, WHITE);
    yOffset += lineHeight;

    // Petit indicateur pour fermer le menu
    DrawText("Press M to hide this menu", menuX + menuWidth - 150, menuY + menuHeight - 20, 14, YELLOW);
}

void UI::drawHelp() {
    int actualScreenWidth = GetScreenWidth();
    int actualScreenHeight = GetScreenHeight();

    int helpWidth = 700;
    int helpHeight = 550;
    int helpX = (actualScreenWidth - helpWidth) / 2;
    int helpY = (actualScreenHeight - helpHeight) / 2;

    // Panel de fond semi-transparent sur tout l'écran
    DrawRectangle(0, 0, actualScreenWidth, actualScreenHeight, ColorAlpha(BLACK, 0.7f));

    // Panel de l'aide
    DrawRectangle(helpX, helpY, helpWidth, helpHeight, ColorAlpha(DARKGRAY, 0.9f));
    DrawRectangleLines(helpX, helpY, helpWidth, helpHeight, WHITE);

    // Titre
    DrawText("Zappy GUI - Help & Controls", helpX + 20, helpY + 20, 28, WHITE);

    // Commandes
    int yOffset = helpY + 70;
    int lineHeight = 25;
    int colWidth = 280;

    // Colonne 1
    DrawText("Camera Controls:", helpX + 20, yOffset, 18, YELLOW);
    yOffset += 30;

    DrawText("WASD / Arrow Keys", helpX + 40, yOffset, 16, WHITE);
    DrawText("Move Camera", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight;

    DrawText("Mouse Wheel", helpX + 40, yOffset, 16, WHITE);
    DrawText("Zoom In / Out", helpX + 40 + colWidth, yOffset, 16, GRAY);
    yOffset += lineHeight * 1.5f;

    // Interface Controls
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

    // Information sur l'application
    DrawText("About:", helpX + 20, yOffset, 18, YELLOW);
    yOffset += 30;

    DrawText("Zappy GUI - A graphical visualization tool", helpX + 40, yOffset, 16, WHITE);
    yOffset += lineHeight;

    // Instructions pour fermer
    DrawText("Press H or ESC to close this window", helpX + helpWidth/2 - 140, helpY + helpHeight - 30, 16, WHITE);
}