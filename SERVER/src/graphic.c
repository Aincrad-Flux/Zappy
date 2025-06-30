/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** graphic.c
*/

#include "server.h"
#include "command/gui_commands.h"

void send_graphic_init_data(server_t *server, int graphic_fd)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "msz %d %d\n", server->width,
        server->height);
    send(graphic_fd, buffer, strlen(buffer), 0);
    handle_gui_mct(server, graphic_fd);
    handle_gui_tna(server, graphic_fd);
    send_player_info(server, graphic_fd);
    snprintf(buffer, sizeof(buffer), "sgt %d\n", server->freq);
    send(graphic_fd, buffer, strlen(buffer), 0);
}

void verif_graphic_connexion(server_t *server, int client_socket, char
    *buffer)
{
    if (strcmp(buffer, "GRAPHIC") == 0) {
        printf("Client GRAPHIC connecté\n");
        server->graphic_fd = client_socket;
        send_graphic_init_data(server, client_socket);
    } else {
        handle_team_authentication(server, client_socket, buffer);
    }
}
