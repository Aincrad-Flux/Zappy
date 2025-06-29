/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** take.c
*/

#include "server.h"
#include "player.h"
#include "command/gui_commands.h"

static void check_resource(tile_t *tile, player_t *player, char *response,
    const char *item)
{
    int id = get_resource_id(item);
    printf("item %s\n", item);
    printf("take %d\n",  id);
    if (id == -1) {
        strcpy(response, "ko\n");
        return;
    }
    if (tile->resources[id] > 0) {
        tile->resources[id]--;
        player->inventory[id]++;
    } else {
        strcpy(response, "ko\n");
    }
}

void handle_take_command(player_t *player, server_t *server, const char *item,
    char *response)
{
    tile_t *tile = &server->map->tiles[player->y][player->x];
    int resource_id = get_resource_id(item);
    int player_id = player - server->players;

    if (resource_id == -1) {
        strcpy(response, "ko\n");
        return;
    }
    strcpy(response, "ok\n");
    check_resource(tile, player, response, item);
    if (strcmp(response, "ok\n") == 0)
        send_gui_pgt(server, player_id, resource_id);
}
