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

static void count_tile(tile_t *tile, int *counts)
{
    for (int i = 0; i < RESOURCE_COUNT; i++) {
        counts[i] += tile->resources[i];
    }
}

static void count_resources(map_t *map, int *counts)
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

static void delete_player_on_tile(list_t *prev, tile_t *tile, list_t *curr)
{
    if (prev)
        prev->next = curr->next;
    else
        tile->players_on_tile = curr->next;
}

void remove_player_from_tile(tile_t *tile, player_t *player)
{
    list_t *prev = NULL;
    list_t *curr = tile->players_on_tile;

    while (curr) {
        if (curr->player == player) {
            delete_player_on_tile(prev, tile, curr);
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void add_player_to_tile(tile_t *tile, player_t *player)
{
    list_t *node = malloc(sizeof(list_t));

    if (!node)
        return;
    node->player = player;
    node->next = tile->players_on_tile;
    tile->players_on_tile = node;
}

tile_t *get_tile(map_t *map, int x, int y)
{
    x = (x + map->width) % map->width;
    y = (y + map->height) % map->height;
    return &map->tiles[y][x];
}
