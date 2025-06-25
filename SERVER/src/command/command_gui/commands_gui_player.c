/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** commands_gui_player
*/

#include "../../../../include/server/server.h"
#include "../../../../include/server/command/gui_commands.h"
#include "../../../../include/server/map/resource.h"

void send_gui_pgt(server_t *server, int player_id, int resource)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "pgt #%d %d\n", player_id, resource);
    broadcast_to_gui_clients(server, buffer);
}

void send_gui_pdi(server_t *server, int player_id)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "pdi #%d\n", player_id);
    broadcast_to_gui_clients(server, buffer);
}

void send_gui_ppo(server_t *server, int player_id)
{
    char buffer[256];
    player_t *player;

    if (player_id < 0 || player_id >= server->num_players) {
        return;
    }
    player = &server->players[player_id];
    snprintf(buffer, sizeof(buffer), "ppo #%d %d %d %d\n",
        player_id, player->x, player->y, player->orientation + 1);
    broadcast_to_gui_clients(server, buffer);
}
