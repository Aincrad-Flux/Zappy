/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** take.c
*/

#include "../../../../include/server/server.h"
#include "../../../../include/server/player.h"
#include "../../../../include/server/command/gui_commands.h"

void handle_take_command(Player *player, Server *server, const char* item,
    char *response)
{
    Tile *tile = &server->map->tiles[player->y][player->x];
    int resource_id = get_resource_id(item);
    int player_id = player - server->players;

    strcpy(response, "ok\n");
    if (strcmp(item, "food") == 0 && tile->food > 0) {
        tile->food--;
        player->food++;
    } else if (strcmp(item, "linemate") == 0 && tile->linemate > 0) {
        tile->linemate--;
        player->linemate++;
    } else if (strcmp(item, "deraumere") == 0 && tile->deraumere > 0) {
        tile->deraumere--;
        player->deraumere++;
    } else if (strcmp(item, "sibur") == 0 && tile->sibur > 0) {
        tile->sibur--;
        player->sibur++;
    } else if (strcmp(item, "mendiane") == 0 && tile->mendiane > 0) {
        tile->mendiane--;
        player->mendiane++;
    } else if (strcmp(item, "phiras") == 0 && tile->phiras > 0) {
        tile->phiras--;
        player->phiras++;
    } else if (strcmp(item, "thystame") == 0 && tile->thystame > 0) {
        tile->thystame--;
        player->thystame++;
    } else {
        strcpy(response, "ko\n");
        return;
    }
    if (resource_id != -1) {
        send_gui_pgt(server, player_id, resource_id);
    }
}
