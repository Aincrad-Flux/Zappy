/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** eject.c
*/


#include "../../../../include/server/server.h"
#include "../../../../include/server/player.h"

static void choose_direction(Player *player, Player *eject, Server *server,
    char *response)
{
    if (player->orientation == 0) {
        strcpy(response, "ok\n");
        eject->y = (eject->y - 1 + server->height) % server->height;
    }
    if (player->orientation == 1) {
        strcpy(response, "ok\n");
        eject->x = (eject->x + 1) % server->width;
    }
    if (player->orientation == 2) {
        eject->y = (eject->y + 1) % server->height;
        strcpy(response, "ok\n");
    }
    if (player->orientation == 3) {
        strcpy(response, "ok\n");
        eject->x = (eject->x - 1 + server->width) % server->width;
    }
}

void handle_eject_command(Player *player, Server *server, char *response)
{
    Tile *tile = &server->map->tiles[player->x][player->y];
    List *list = tile->players_on_tile;

    strcpy(response, "ko\n");
    while (list != NULL) {
        choose_direction(player, list->player, server, response);
        list = list->next;
    }
}