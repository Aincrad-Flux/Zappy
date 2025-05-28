#pragma once

#include <raylib.h>
#include <vector>
#include <string>
#include "Player.hpp"

class UI {
private:
    int screenWidth;
    int screenHeight;
    Font font;
    Player* selectedPlayer;
    bool showPlayerInfo;
    bool showTeamStats;
    std::vector<std::string> teams;

    void drawPlayerInfo();
    void drawTeamStats();
    void drawResourceLegend();
    void drawGameStats();

public:
    UI(int width, int height);
    ~UI();

    void draw(const std::vector<Player>& players);
    void handleInput();
    void setSelectedPlayer(Player* player);
    void addTeam(const std::string& teamName);
    void togglePlayerInfo();
    void toggleTeamStats();
};