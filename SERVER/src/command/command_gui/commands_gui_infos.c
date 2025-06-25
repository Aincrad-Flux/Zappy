/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** commands_gui_infos
*/

#include "server.h"
#include "command/gui_commands.h"
#include "map/resource.h"

static int validate_player_id(server_t *server, int player_id)
{
    return (player_id >= 0 && player_id < server->num_players);
}

void handle_gui_ppo(server_t *server, int client_socket, int player_id)
{
    char buffer[256];
    player_t *player;

    if (!validate_player_id(server, player_id)) {
        send(client_socket, "sbp\n", 4, 0);
        return;
    }
    player = &server->players[player_id];
    snprintf(buffer, sizeof(buffer), "ppo #%d %d %d %d\n",
        player_id, player->x, player->y, player->orientation + 1);
    send(client_socket, buffer, strlen(buffer), 0);
}

void handle_gui_plv(server_t *server, int client_socket, int player_id)
{
    char buffer[256];
    player_t *player;

    if (!validate_player_id(server, player_id)) {
        send(client_socket, "sbp\n", 4, 0);
        return;
    }
    player = &server->players[player_id];
    snprintf(buffer, sizeof(buffer), "plv #%d %d\n",
        player_id, player->level);
    send(client_socket, buffer, strlen(buffer), 0);
}

static void format_pin_response(char *buffer, int player_id, player_t *player)
{
    snprintf(buffer, 256, "pin #%d %d %d %d %d %d %d %d %d %d\n",
        player_id, player->x, player->y, player->inventory[FOOD],
            player->inventory[LINEMATE], player->inventory[DERAUMERE],
                player->inventory[SIBUR], player->inventory[MENDIANE],
                    player->inventory[PHIRAS],
                        player->inventory[THYSTAME]);
}

void handle_gui_pin(server_t *server, int client_socket, int player_id)
{
    char buffer[256];
    player_t *player;

    if (!validate_player_id(server, player_id)) {
        send(client_socket, "sbp\n", 4, 0);
        return;
    }
    player = &server->players[player_id];
    format_pin_response(buffer, player_id, player);
    send(client_socket, buffer, strlen(buffer), 0);
}
