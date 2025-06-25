/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** map.h
*/


#ifndef MAP
    #define MAP
    #include "../player.h"

typedef struct List list_t;
struct List {
    player_t *player;
    list_t *next;
};

typedef struct {
    int *resources;
    list_t *players_on_tile;
} tile_t;

typedef struct {
    int width;
    int height;
    tile_t **tiles;
} map_t;

void init_map(server_t *server);

#endif
