/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** set.c
*/


#include "server.h"
#include "player.h"
#include "command/gui_commands.h"

void handle_set_command(player_t *player, server_t *server, const char *item,
    char *response)
{
    tile_t *tile = &server->map->tiles[player->y][player->x];
    int resource_id = get_resource_id(item);
    int player_id = player - server->players;

    if (resource_id == -1 || player->inventory[resource_id] <= 0) {
        strcpy(response, "ko\n");
        return;
    }
    player->inventory[resource_id]--;
    tile->resources[resource_id]++;
    strcpy(response, "ok\n");
    send_gui_pdr(server, player_id, resource_id);
}
