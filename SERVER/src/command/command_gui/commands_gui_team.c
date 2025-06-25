/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** commands_gui_team
*/

#include "server.h"
#include "command/gui_commands.h"

void handle_gui_tna(server_t *server, int client_socket)
{
    char buffer[256];

    for (int i = 0; i < server->num_teams; i++) {
        snprintf(buffer, sizeof(buffer), "tna %s\n",
            server->teams[i].name);
        send(client_socket, buffer, strlen(buffer), 0);
    }
}
