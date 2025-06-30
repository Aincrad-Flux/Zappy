/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** Look.c
*/

#include "server.h"
#include "map/map.h"
#include "map/resource.h"

static void append_tile_content(char *buffer, tile_t *tile)
{
    int first = 1;
    static const char *RESOURCE_NAMES[] = {
            "food", "linemate", "deraumere", "sibur", "mendiane", "phiras",
            "thystame" };

    for (list_t *node = tile->players_on_tile; node != NULL; node =
            node->next) {
        strcat(buffer, first ? "player" : " player");
        first = 0;
    }

    for (int i = 0; i < RESOURCE_COUNT; i++) {
        for (int j = 0; j < tile->resources[i]; j++) {
            if (!first)
                strcat(buffer, " ");
            strcat(buffer, RESOURCE_NAMES[i]);
            first = 0;
        }
    }
}

static tile_t *get_tile(map_t *map, int x, int y)
{
    x = (x + map->width) % map->width;
    y = (y + map->height) % map->height;
    return &map->tiles[y][x];
}

static tile_t *tile_orientation(player_t *player , server_t *server, int depth,
    int offset)
{
    int px = player->x;
    int py = player->y;
    int tx = px;
    int ty = py;

    switch (player->orientation) {
        case NORTH: ty = py - depth; tx = px + offset; break;
        case SOUTH: ty = py + depth; tx = px - offset; break;
        case EAST:  tx = px + depth; ty = py + offset; break;
        case WEST:  tx = px - depth; ty = py - offset; break;
    }
    return get_tile(server->map, tx, ty);
}

void handle_look_command(player_t *player, server_t *server, char *response)
{
    char buffer[BUFFER_SIZE] = {0};
    int level = player->level;
    int len;

    strcat(buffer, "[");
    for (int depth = 0; depth <= level; depth++) {
        for (int offset = -depth; offset <= depth; offset++) {
            append_tile_content(buffer, tile_orientation(player,server,
                    depth, offset));
            strcat(buffer, ",");
        }
    }
    len = strlen(buffer);
    if (len > 1 && buffer[len - 1] == ',')
        buffer[len - 1] = '\0';
    strcat(buffer, "]\n");
    printf("player x %d y %d\n", player->x, player->y);
    printf("look: %s\n", buffer);
    strcpy(response, buffer);
}

