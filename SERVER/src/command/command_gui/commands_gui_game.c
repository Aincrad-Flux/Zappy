/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** commands_gui_game
*/


#include "../../../../include/server/server.h"
#include "../../../../include/server/command/gui_commands.h"

void send_gui_seg(server_t *server, const char *team_name)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "seg %s\n", team_name);
    broadcast_to_gui_clients(server, buffer);
}

void send_gui_smg(server_t *server, const char *message)
{
    char buffer[512];

    snprintf(buffer, sizeof(buffer), "smg %s\n", message);
    broadcast_to_gui_clients(server, buffer);
}
