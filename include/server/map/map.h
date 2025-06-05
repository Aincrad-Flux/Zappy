/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** map.h
*/


#ifndef MAP
#define MAP
#include "../player.h"

typedef struct List List;
struct List{
    Player *player;
    List *next;
};

typedef struct {
    int food;
    int linemate;
    int deraumere;
    int sibur;
    int mendiane;
    int phiras;
    int thystame;
    List *players_on_tile;
} Tile;

typedef struct {
    int width;
    int height;
    Tile **tiles;
} Map;

void init_map(Server *server);

#endif