/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** resource.h
*/

#ifndef RESOURCE
    #define RESOURCE
    #include "map.h"

    #define RESOURCE_COUNT 7

enum Orientation {
    NORTH,
    EAST,
    SOUTH,
    WEST
};

typedef enum {
    FOOD = 0,
    LINEMATE,
    DERAUMERE,
    SIBUR,
    MENDIANE,
    PHIRAS,
    THYSTAME
} resourcetype_t;

void init_ressources(map_t *map);
void respawn_resource(map_t *map);
void add_player_to_tile(tile_t *tile, player_t *player);
void remove_player_from_tile(tile_t *tile, player_t *player);
tile_t *get_tile(map_t *map, int x, int y);

#endif
