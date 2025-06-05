/*
** EPITECH PROJECT, 2025
** start
** File description:
** command.c
*/

#include "../../../include/server/utils/command.h"


int get_command_duration(const char *command)
{
    size_t len = 0;
    static CommandDuration durations[] = {{"Forward", 7}, {"Right", 7},
        {"Left", 7}, {"Look", 7}, {"Inventory", 1},
            {"Take", 7}, {"Set", 7}, {"Eject", 7},
                {"Broadcast", 7}, {"Incantation", 300},
                    {"Fork", 42}, {"Connect_nbr", 0}, {NULL, 0} };

    for (int i = 0; durations[i].name != NULL; i++) {
        len = strlen(durations[i].name);
        if (strncmp(command, durations[i].name, len) == 0 &&
            (command[len] == '\0' || command[len] == ' '))
            return durations[i].duration;
    }
    return -1;
}
