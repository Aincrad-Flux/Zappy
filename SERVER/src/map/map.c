/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** map.c
*/

#include "map/map.h"
#include "server.h"
#include "map/resource.h"

static void init_tile(tile_t *tile)
{
    memset(tile, 0, sizeof(tile_t));
    tile->resources = malloc(sizeof(int) * RESOURCE_COUNT);
    for (int r = 0; r < RESOURCE_COUNT; r++) {
        tile->resources[r] = 0;
    }
    tile->players_on_tile = NULL;
}

static void init_map_row(tile_t *row, int width)
{
    for (int x = 0; x < width; x++) {
        init_tile(&row[x]);
    }
}

void init_map(server_t *server)
{
    map_t *map = malloc(sizeof(map_t));

    map->width = server->width;
    map->height = server->height;
    map->tiles = malloc(sizeof(tile_t *) * map->height);
    for (int y = 0; y < map->height; y++) {
        map->tiles[y] = malloc(sizeof(tile_t) * map->width);
        init_map_row(map->tiles[y], map->width);
    }
    init_ressources(map);
    server->map = map;
}
