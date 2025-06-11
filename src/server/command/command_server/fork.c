/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** fork.c
*/

#include "../../../../include/server/server.h"
#include "../../../../include/server/player.h"
#include "../../../../include/server/command/gui_commands.h"

void handle_fork_command(Player *player, Server *server, char *response)
{
    int player_id = player - server->players;
    int egg_id = server->next_egg_id++;

    strcpy(response, "ok\n");

    // Add slot to team
    server->teams[player->team_id].max_clients++;

    // Notify GUI clients
    send_gui_pfk(server, player_id);
    send_gui_enw(server, egg_id, player_id, player->x, player->y);
}