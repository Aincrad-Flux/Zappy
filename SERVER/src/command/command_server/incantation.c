/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** incantation.c
*/

#include "server.h"
#include "player.h"
#include "map/resource.h"

static tile_t *get_tile(map_t *map, int x, int y)
{
    x = (x + map->width) % map->width;
    y = (y + map->height) % map->height;
    return &map->tiles[y][x];
}

bool has_required_resources(tile_t *tile, int level)
{
    const int elevation_requirements[8][RESOURCE_COUNT + 1] = {
            {0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 0, 0, 0, 0, 2},
            {2, 0, 1, 0, 2, 0, 0, 2},
            {1, 1, 2, 0, 1, 0, 0, 4},
            {1, 2, 1, 3, 0, 0, 0, 4},
            {1, 2, 3, 0, 1, 0, 0, 6},
            {2, 2, 2, 2, 2, 1, 0, 6}
    };

    for (int i = 0; i < RESOURCE_COUNT; i++) {
        if (tile->resources[i] < elevation_requirements[level][i])
            return false;
    }
    return true;
}

int count_same_level_players(tile_t *tile, int level)
{
    int count = 0;

    for (list_t *node = tile->players_on_tile; node; node = node->next) {
        if (node->player && node->player->level == level)
            count++;
    }
    return count;
}

void prepare_incantation(player_t *player, server_t *server, char *response)
{
    tile_t *tile = get_tile(server->map, player->x, player->y);
    int level = player->level;
    const int elevation_requirements[8][RESOURCE_COUNT + 1] = {
            {0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 0, 0, 0, 0, 2},
            {2, 0, 1, 0, 2, 0, 0, 2},
            {1, 1, 2, 0, 1, 0, 0, 4},
            {1, 2, 1, 3, 0, 0, 0, 4},
            {1, 2, 3, 0, 1, 0, 0, 6},
            {2, 2, 2, 2, 2, 1, 0, 6}
    };
    int required_players = elevation_requirements[level][RESOURCE_COUNT];

    if (level >= 8 || !has_required_resources(tile, level) ||
        count_same_level_players(tile, level) < required_players) {
        strcpy(response, "ko\n");
        return;
    }

    for (list_t *node = tile->players_on_tile; node; node = node->next) {
        if (node->player && node->player->level == level) {
            node->player->is_incanting = true;
            node->player->is_waiting_level_up = true;
        }
    }
    add_action_to_queue(player, "Incantation", server->freq);
    strcpy(response, "Elevation underway\n");
}

void finish_incantation(player_t *player, server_t *server)
{
    int level = player->level;
    tile_t *tile = get_tile(server->map, player->x, player->y);
    const int elevation_requirements[8][RESOURCE_COUNT + 1] = {
            {0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 0, 0, 0, 0, 2},
            {2, 0, 1, 0, 2, 0, 0, 2},
            {1, 1, 2, 0, 1, 0, 0, 4},
            {1, 2, 1, 3, 0, 0, 0, 4},
            {1, 2, 3, 0, 1, 0, 0, 6},
            {2, 2, 2, 2, 2, 1, 0, 6}
    };
    int required_players = elevation_requirements[level][RESOURCE_COUNT];

    if (!has_required_resources(tile, level) ||
        count_same_level_players(tile, level) < required_players) {
        for (list_t *node = tile->players_on_tile; node; node = node->next) {
            player_t *p = node->player;
            if (p && p->is_waiting_level_up && p->level == level) {
                p->is_waiting_level_up = false;
                p->is_incanting = false;
                dprintf(p->socket, "ko\n");
            }
        }
        return;
    }
    for (list_t *node = tile->players_on_tile; node; node = node->next) {
        player_t *p = node->player;
        if (p && p->is_waiting_level_up && p->level == level) {
            p->level++;
            p->is_waiting_level_up = false;
            p->is_incanting = false;
            dprintf(p->socket, "Current level: %d\n", p->level);
        }
    }
    for (int i = 0; i < RESOURCE_COUNT; i++)
        tile->resources[i] -= elevation_requirements[level][i];
}