/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** commands_gui_egg
*/

#include "server.h"
#include "command/gui_commands.h"

void send_gui_enw(server_t *server, int egg_id, int player_id, position_t pos)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "enw #%d #%d %d %d\n",
        egg_id, player_id, pos.x, pos.y);
    broadcast_to_gui_clients(server, buffer);
}

void send_gui_ebo(server_t *server, int egg_id)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "ebo #%d\n", egg_id);
    broadcast_to_gui_clients(server, buffer);
}

void send_gui_edi(server_t *server, int egg_id)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "edi #%d\n", egg_id);
    broadcast_to_gui_clients(server, buffer);
}
