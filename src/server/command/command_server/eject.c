/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** eject.c
*/


#include "../../../../include/server/server.h"
#include "../../../../include/server/player.h"
#include "../../../../include/server/command/gui_commands.h"

static void move_player_direction(Player *player, Server *server, int dir)
{
    switch (dir) {
        case 0: // North
            player->y = (player->y - 1 + server->height) % server->height;
            break;
        case 1: // East
            player->x = (player->x + 1) % server->width;
            break;
        case 2: // South
            player->y = (player->y + 1) % server->height;
            break;
        case 3: // West
            player->x = (player->x - 1 + server->width) % server->width;
            break;
    }
}

void handle_eject_command(Player *player, Server *server, char *response)
{
    Tile *tile = &server->map->tiles[player->y][player->x];
    List *list = tile->players_on_tile;
    int player_id = player - server->players;
    int ejected = 0;

    while (list != NULL) {
        if (list->player != player) {
            move_player_direction(list->player, server, player->orientation);

            // Send eject notification to ejected player
            char eject_msg[64];
            int reverse_dir = (player->orientation + 2) % 4 + 1;
            snprintf(eject_msg, sizeof(eject_msg), "eject: %d\n", reverse_dir);
            send(list->player->socket, eject_msg, strlen(eject_msg), 0);

            ejected = 1;
        }
        list = list->next;
    }

    strcpy(response, ejected ? "ok\n" : "ko\n");

    if (ejected) {
        send_gui_pex(server, player_id);
    }
}
