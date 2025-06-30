/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** forward.c
*/

#include "server.h"
#include "player.h"
#include "command/gui_commands.h"
#include "map/resource.h"


static void new_pos(server_t *server, player_t *player)
{
    tile_t *tile = get_tile(server->map, player->x, player->y);

    add_player_to_tile(tile, player);
}

void move_player_forward(player_t *player, server_t *server)
{
    int player_id = player - server->players;
    tile_t *tile = get_tile(server->map, player->x, player->y);

    remove_player_from_tile(tile, player);
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
    new_pos(server, player);
    send_gui_ppo(server, player_id);
}
