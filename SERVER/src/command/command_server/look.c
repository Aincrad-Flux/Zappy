/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** look.c
*/

#include "server.h"
#include "map/map.h"
#include "map/resource.h"

static void append_space_if_needed(char *buffer, int *first)
{
    if (!(*first))
        strcat(buffer, " ");
}

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
            append_space_if_needed(buffer, &first);
            strcat(buffer, RESOURCE_NAMES[i]);
            first = 0;
        }
    }
}

static tile_t *handle_north_orientation(player_t *player, server_t *server,
    int depth, int offset)
{
    int tx = player->x + offset;
    int ty = player->y - depth;

    return get_tile(server->map, tx, ty);
}

static tile_t *handle_south_orientation(player_t *player, server_t *server,
    int depth, int offset)
{
    int tx = player->x - offset;
    int ty = player->y + depth;

    return get_tile(server->map, tx, ty);
}

static tile_t *handle_east_orientation(player_t *player, server_t *server,
    int depth, int offset)
{
    int tx = player->x + depth;
    int ty = player->y + offset;

    return get_tile(server->map, tx, ty);
}

static tile_t *handle_west_orientation(player_t *player, server_t *server,
    int depth, int offset)
{
    int tx = player->x - depth;
    int ty = player->y - offset;

    return get_tile(server->map, tx, ty);
}

static tile_t *tile_orientation(player_t *player, server_t *server, int depth,
    int offset)
{
    if (player->orientation == NORTH)
        return handle_north_orientation(player, server, depth, offset);
    if (player->orientation == SOUTH)
        return handle_south_orientation(player, server, depth, offset);
    if (player->orientation == EAST)
        return handle_east_orientation(player, server, depth, offset);
    return handle_west_orientation(player, server, depth, offset);
}

static void remove_trailing_comma(char *buffer)
{
    int len = strlen(buffer);

    if (len > 1 && buffer[len - 1] == ',')
        buffer[len - 1] = '\0';
}

void handle_look_command(player_t *player, server_t *server, char *response)
{
    char buffer[BUFFER_SIZE] = {0};
    int level = player->level;

    strcat(buffer, "[");
    for (int depth = 0; depth <= level; depth++) {
        for (int offset = -depth; offset <= depth; offset++) {
            append_tile_content(buffer, tile_orientation(player, server,
                    depth, offset));
            strcat(buffer, ",");
        }
    }
    remove_trailing_comma(buffer);
    strcat(buffer, "]\n");
    printf("player x %d y %d\n", player->x, player->y);
    printf("look: %s\n", buffer);
    strcpy(response, buffer);
}
