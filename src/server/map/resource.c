/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** resource.c
*/

#include "../../../include/server/map/resource.h"

void add_resource(Tile *tile, ResourceType type)
{
    if (type == FOOD)
        tile->food++;
    if (type == LINEMATE)
        tile->linemate++;
    if (type == DERAUMERE)
        tile->deraumere++;
    if (type == SIBUR)
        tile->sibur++;
    if (type == MENDIANE)
        tile->mendiane++;
    if (type == PHIRAS)
        tile->phiras++;
    if (type == THYSTAME)
        tile->thystame++;
}

void distribute_resource(Map *map, int count, ResourceType type)
{
    int x = 0;
    int y = 0;
    Tile *tile = NULL;

    for (int i = 0; i < count; i++) {
        x = rand() % map->width;
        y = rand() % map->height;
        tile = &map->tiles[x][y];
        add_resource(tile, type);
    }
}

void init_ressources(Map *map)
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

void count_resources(Map *map, int *counts)
{
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            Tile *tile = &map->tiles[y][x];
            counts[FOOD]     += tile->food;
            counts[LINEMATE] += tile->linemate;
            counts[DERAUMERE] += tile->deraumere;
            counts[SIBUR]    += tile->sibur;
            counts[MENDIANE] += tile->mendiane;
            counts[PHIRAS]   += tile->phiras;
            counts[THYSTAME] += tile->thystame;
        }
    }
}

void respawn_resource(Map *map)
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