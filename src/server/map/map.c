/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** map.c
*/

#include "../../../include/server/map/map.h"
#include "../../../include/server/server.h"
#include "../../../include/server/map/resource.h"

void init_map(Server *server)
{
    Map *map = malloc(sizeof(Map));

    map->width = server->width;
    map->height = server->height;
    map->tiles = malloc(sizeof(Tile*) * map->height);
    for (int y = 0; y < map->height; y++) {
        map->tiles[y] = malloc(sizeof(Tile) * map->width);
        for (int x = 0; x < map->width; x++) {
            memset(&map->tiles[y][x], 0, sizeof(Tile));
        }
    }
    init_ressources(map);
    server->map = map;
}


