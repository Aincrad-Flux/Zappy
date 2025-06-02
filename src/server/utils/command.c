/*
** EPITECH PROJECT, 2025
** start
** File description:
** command.c
*/

#include "../../../include/server/utils/command.h"


int get_command_duration(const char *command)
{
     static CommandDuration durations[] = {{"avance", 7}, {"droite", 7},
            {"gauche", 7}, {"voir", 7}, {"inventaire", 1}, {"prend", 7},
            {"pose", 7}, {"expulse", 7}, {"broadcast", 7}, {"incantation", 300},
            {"fork", 42}, {"connect_nbr", 0}, {NULL, 0}
    };

    for (int i = 0; durations[i].name != NULL; i++) {
        if (strcmp(command, durations[i].name) == 0)
            return durations[i].duration;
    }
    return -1;
}

