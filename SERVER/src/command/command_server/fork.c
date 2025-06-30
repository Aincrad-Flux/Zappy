/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** fork.c
*/

#include "server.h"
#include "player.h"
#include "command/gui_commands.h"

void handle_fork_command(player_t *player, server_t *server, char *response)
{
    position_t pos = {player->x, player->y};
    int player_id = 0;
    int egg_id = 0;

    player_id = player - server->players;
    egg_id = server->next_egg_id;
    server->next_egg_id++;
    strcpy(response, "ok\n");
    server->teams[player->team_id].max_clients++;
    send_gui_pfk(server, player_id);
    send_gui_enw(server, egg_id, player_id, pos);
}
