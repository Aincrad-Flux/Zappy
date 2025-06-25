/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** command_gui_extended_2
*/

#include "server.h"
#include "command/gui_commands.h"
#include "map/resource.h"

void send_gui_pie(server_t *server, int x, int y, int result)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "pie %d %d %d\n", x, y, result);
    broadcast_to_gui_clients(server, buffer);
}

void send_gui_pfk(server_t *server, int player_id)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "pfk #%d\n", player_id);
    broadcast_to_gui_clients(server, buffer);
}

void send_gui_pdr(server_t *server, int player_id, int resource)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "pdr #%d %d\n", player_id, resource);
    broadcast_to_gui_clients(server, buffer);
}
