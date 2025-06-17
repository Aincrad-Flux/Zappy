/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** broadcast.c
*/
#include "../../../../include/server/server.h"
#include "../../../../include/server/player.h"
#include "../../../../include/server/command/gui_commands.h"

void handle_broadcast_command(Player *player, Server *server, const char *
    message, char *response)
{
    int player_id = player - server->players;
    char broadcast_msg[512];
    int direction;

    strcpy(response, "ok\n");
    for (int i = 0; i < server->num_players; i++) {
        if (i != player_id) {
            direction = calculate_sound_direction(player,
                &server->players[i], server);
            snprintf(broadcast_msg, sizeof(broadcast_msg),
                "message %d, %s\n", direction, message);
            send(server->players[i].socket, broadcast_msg,
                strlen(broadcast_msg), 0);
        }
    }
    send_gui_pbc(server, player_id, message);
}
