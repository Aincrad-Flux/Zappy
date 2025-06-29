/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** forward.c
*/

#include "server.h"
#include "player.h"
#include "command/gui_commands.h"

void move_player_forward(player_t *player, server_t *server)
{
    int player_id = player - server->players;

    switch (player->orientation) {
        case 0:
            player->y = (player->y - 1 + server->height) % server->height;
            break;
        case 1:
            player->x = (player->x + 1) % server->width;
            break;
        case 2:
            player->y = (player->y + 1) % server->height;
            break;
        case 3:
            player->x = (player->x - 1 + server->width) % server->width;
            break;
    }
    printf("played id %d x %d y %d", player_id, player->x, player->y);
    send_gui_ppo(server, player_id);
}
