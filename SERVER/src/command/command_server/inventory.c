/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** inventory.c
*/

#include "server.h"
#include "player.h"
#include "map/resource.h"

void handle_inventory_command(player_t *player, char *response)
{
    snprintf(response, BUFFER_SIZE,
        "[ food %d, linemate %d, deraumere %d, sibur %d, "
            "mendiane %d, phiras %d, thystame %d ]\n",
                player->inventory[FOOD], player->inventory[LINEMATE],
                    player->inventory[DERAUMERE], player->inventory[SIBUR],
                        player->inventory[MENDIANE], player->inventory[PHIRAS],
                            player->inventory[THYSTAME]
    );
}
