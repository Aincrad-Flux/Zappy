/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** broadcast_to_gui_clients
*/

#include "server.h"

void send_to_gui_client(int socket, const char *message)
{
    ssize_t result = send(socket, message, strlen(message), 0);
    printf("reussi\n");
    if (result == -1) {
        printf("Erreur envoi vers client GUI socket %d\n", socket);
    }
}

void broadcast_to_gui_clients(server_t *server, const char *message)
{
    for (int i = 0; i < server->num_players; i++) {
        if (server->players[i].team_id == -1) {
            send_to_gui_client(server->graphic_fd, message);
        }
    }
}
