/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** commands_gui_map
*/

#include "../../../../include/server/server.h"
#include "../../../../include/server/command/gui_commands.h"
#include "../../../../include/server/map/resource.h"

void handle_gui_msz(server_t *server, int client_socket)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "msz %d %d\n",
        server->width, server->height);
    send(client_socket, buffer, strlen(buffer), 0);
}

void handle_gui_bct(server_t *server, int client_socket, int x, int y)
{
    char buffer[256];
    tile_t *tile;

    if (x < 0 || x >= server->width || y < 0 || y >= server->height) {
        send(client_socket, "sbp\n", 4, 0);
        return;
    }
    tile = &server->map->tiles[y][x];
    snprintf(buffer, sizeof(buffer), "bct %d %d %d %d %d %d %d %d %d\n",
        x, y, tile->resources[FOOD], tile->resources[LINEMATE],
            tile->resources[DERAUMERE], tile->resources[SIBUR],
                tile->resources[MENDIANE], tile->resources[PHIRAS],
                    tile->resources[THYSTAME]);
    send(client_socket, buffer, strlen(buffer), 0);
}

void handle_gui_mct(server_t *server, int client_socket)
{
    for (int y = 0; y < server->height; y++) {
        for (int x = 0; x < server->width; x++) {
            handle_gui_bct(server, client_socket, x, y);
        }
    }
}
