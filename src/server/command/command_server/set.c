/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** set.c
*/

#include "../../../../include/server/server.h"
#include "../../../../include/server/player.h"

void handle_set_command(Player *player, Server *server, const char* item,
                         char *response)
{
    Tile *tile = &server->map->tiles[player->x][player->y];

    strcpy(response, "ok\n");
    if (strcmp(item, "food") == 0 && player->food > 0) {
        tile->food++;
        player->food--;
    } else if (strcmp(item, "linemate") == 0 && player->linemate > 0) {
        tile->linemate++;
        player->linemate--;
    } else if (strcmp(item, "deraumere") == 0 && player->deraumere > 0) {
        tile->deraumere++;
        player->deraumere--;
    } else if (strcmp(item, "sibur") == 0 && player->sibur > 0) {
        tile->sibur++;
        player->sibur--;
    } else if (strcmp(item, "mendiane") == 0 && player->mendiane > 0) {
        tile->mendiane++;
        player->mendiane--;
    } else if (strcmp(item, "phiras") == 0 && player->phiras > 0) {
        tile->phiras++;
        player->phiras--;
    } else if (strcmp(item, "thystame") == 0 && player->thystame > 0) {
        tile->thystame++;
        player->thystame--;
    } else
        strcpy(response, "ko\n");
}