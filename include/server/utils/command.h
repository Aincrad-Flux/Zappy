/*
** EPITECH PROJECT, 2025
** start
** File description:
** command.h
*/

#ifndef COMMAND
#define COMMAND

#include <string.h>

typedef struct {
    const char *name;
    int duration;
} CommandDuration;

int get_command_duration(const char *command);

#endif