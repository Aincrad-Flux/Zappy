/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** commands_gui_utils
*/

#include "../../../../include/server/server.h"
#include "../../../../include/server/command/gui_commands.h"

int get_resource_id(const char *resource_name)
{
    static const char *resources[] = {
        "food", "linemate", "deraumere", "sibur",
        "mendiane", "phiras", "thystame", NULL
    };

    for (int i = 0; resources[i] != NULL; i++) {
        if (strcmp(resource_name, resources[i]) == 0) {
            return i;
        }
    }
    return -1;
}
