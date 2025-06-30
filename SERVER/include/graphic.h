/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** graphic.h
*/

#ifndef GRAPHIC
    #define GRAPHIC

typedef struct Server server_t;

void send_graphic_init_data(server_t *server, int graphic_fd);
void verif_graphic_connexion(server_t *server, int client_socket, char
    *buffer);

#endif
