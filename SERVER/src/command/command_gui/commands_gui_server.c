/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** commands_gui_server
*/

#include "../../../../include/server/server.h"
#include "../../../../include/server/command/gui_commands.h"

void handle_gui_sgt(server_t *server, int client_socket)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "sgt %d\n", server->freq);
    send(client_socket, buffer, strlen(buffer), 0);
}
