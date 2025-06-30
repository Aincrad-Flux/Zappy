/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** incantation.c
*/

#include "server.h"
#include "player.h"
#include "map/resource.h"
#include "command/gui_commands.h"


bool has_required_resources(tile_t *tile, int level, const int
    elevation_requirements[][8])
{
    for (int i = 0; i < RESOURCE_COUNT; i++) {
        if (tile->resources[i] < elevation_requirements[level][i]) {
            return false;
        }
    }
    return true;
}

int count_same_level_players(tile_t *tile, int level)
{
    int count = 0;

    for (list_t *node = tile->players_on_tile; node; node = node->next) {
        printf("player %d\n", node->player->level);
        if (node->player && node->player->level == level)
            count++;
    }
    return count;
}

static void start_level_up(tile_t *tile, int level)
{
    for (list_t *node = tile->players_on_tile; node; node = node->next) {
        if (node->player && node->player->level == level) {
            node->player->is_incanting = true;
            node->player->is_waiting_level_up = true;
        }
    }
}

void prepare_incantation(player_t *player, server_t *server, char *response)
{
    tile_t *tile = get_tile(server->map, player->x, player->y);
    int level = player->level;
    const int elevation_requirements[8][RESOURCE_COUNT + 1] = {
            {0, 0, 0, 0, 0, 0, 0, 1}, {0, 1, 0, 0, 0, 0, 0, 1},
            {0, 1, 1, 1, 0, 0, 0, 2}, {0, 2, 0, 1, 0, 2, 0, 2},
            {0, 1, 1, 2, 0, 1, 0, 4}, {0, 1, 2, 1, 3, 0, 0, 4},
            {0, 1, 2, 3, 0, 1, 0, 6}, {0, 2, 2, 2, 2, 2, 1, 6}
    };
    int required_players = elevation_requirements[level][RESOURCE_COUNT];

    if (level >= 8 || !has_required_resources(tile, level,
        elevation_requirements) || count_same_level_players(tile, level) <
        required_players) {
        strcpy(response, "ko\n");
        return;
    }
    start_level_up(tile, level);
    add_action_to_queue(player, "Incantation", server->freq);
    strcpy(response, "Elevation underway\n");
    //send_gui_pic(server, player->x, player->y, 1);
}

static void cancel_incantation(tile_t *tile, int level)
{
    player_t *p;

    for (list_t *node = tile->players_on_tile; node; node = node->next) {
        p = node->player;
        if (p && p->is_waiting_level_up && p->level == level) {
            p->is_waiting_level_up = false;
            p->is_incanting = false;
            dprintf(p->socket, "ko\n");
        }
    }
}

static void increase_level(tile_t *tile, int level)
{
    player_t *p;

    for (list_t *node = tile->players_on_tile; node; node = node->next) {
        p = node->player;
        if (p && p->is_waiting_level_up && p->level == level) {
            p->level++;
            p->is_waiting_level_up = false;
            p->is_incanting = false;
            dprintf(p->socket, "Current level: %d\n", p->level);
        }
    }
}

void finish_incantation(player_t *player, server_t *server)
{
    int level = player->level;
    tile_t *tile = get_tile(server->map, player->x, player->y);
    const int elevation_requirements[8][RESOURCE_COUNT + 1] = {
            {0, 0, 0, 0, 0, 0, 0, 1}, {0, 1, 0, 0, 0, 0, 0, 1},
            {0, 1, 1, 1, 0, 0, 0, 2}, {0, 2, 0, 1, 0, 2, 0, 2},
            {0, 1, 1, 2, 0, 1, 0, 4}, {0, 1, 2, 1, 3, 0, 0, 4},
            {0, 1, 2, 3, 0, 1, 0, 6}, {0, 2, 2, 2, 2, 2, 1, 6}
    };
    int required_players = elevation_requirements[level][RESOURCE_COUNT];

    if (!has_required_resources(tile, level, elevation_requirements) ||
        count_same_level_players(tile, level) < required_players) {
        cancel_incantation(tile, level);
        send_gui_pie(server, player->x, player->y, 0);
        return;
    }
    increase_level(tile, level);
    for (int i = 0; i < RESOURCE_COUNT; i++)
        tile->resources[i] -= elevation_requirements[level][i];
    send_gui_pie(server, player->x, player->y, 1);
}
