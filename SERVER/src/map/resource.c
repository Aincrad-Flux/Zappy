/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** resource
*/

#include "map/resource.h"

static void add_resource(tile_t *tile, resourcetype_t type)
{
    if (type >= 0 && type < RESOURCE_COUNT) {
        tile->resources[type]++;
    }
}

static void distribute_resource(map_t *map, int count, resourcetype_t type)
{
    int x = 0;
    int y = 0;
    tile_t *tile = NULL;

    for (int i = 0; i < count; i++) {
        x = rand() % map->width;
        y = rand() % map->height;
        tile = &map->tiles[y][x];
        add_resource(tile, type);
    }
}

void init_ressources(map_t *map)
{
    int num_tiles = map->width * map->height;

    srand(time(NULL));
    distribute_resource(map, num_tiles * 0.5, FOOD);
    distribute_resource(map, num_tiles * 0.3, LINEMATE);
    distribute_resource(map, num_tiles * 0.15, DERAUMERE);
    distribute_resource(map, num_tiles * 0.1, SIBUR);
    distribute_resource(map, num_tiles * 0.1, MENDIANE);
    distribute_resource(map, num_tiles * 0.08, PHIRAS);
    distribute_resource(map, num_tiles * 0.05, THYSTAME);
}

void count_tile(tile_t *tile, int *counts)
{
    for (int i = 0; i < RESOURCE_COUNT; i++) {
        counts[i] += tile->resources[i];
    }
}

void count_resources(map_t *map, int *counts)
{
    tile_t *tile;

    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            tile = &map->tiles[y][x];
            count_tile(tile, counts);
        }
    }
}

void respawn_resource(map_t *map)
{
    int total_tiles = map->width * map->height;
    int current[7] = {0};
    int missing = 0;
    int expected[7] = {
            total_tiles * 0.5,
            total_tiles * 0.3,
            total_tiles * 0.15,
            total_tiles * 0.1,
            total_tiles * 0.1,
            total_tiles * 0.08,
            total_tiles * 0.05
    };

    count_resources(map, current);
    for (int i = 0; i < 7; i++) {
        missing = expected[i] - current[i];
        if (missing > 0)
            distribute_resource(map, missing, i);
    }
}
