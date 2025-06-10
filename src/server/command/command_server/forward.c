/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** forward.c
*/

#include "../../../../include/server/server.h"
#include "../../../../include/server/player.h"

void move_player_forward(Player *player, Server *server)
{
    switch (player->orientation) {
        case 0: player->y = (player->y - 1 + server->height) % server->height; break;
        case 1: player->x = (player->x + 1) % server->width; break;
        case 2: player->y = (player->y + 1) % server->height; break;
        case 3: player->x = (player->x - 1 + server->width) % server->width; break;
    }
}
