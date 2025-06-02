/*
** EPITECH PROJECT, 2025
** start
** File description:
** action.h
*/

#ifndef ACTION
#define ACTION

#include "time.h"
#include <stdio.h>
#include <math.h>
typedef struct Server Server;
typedef struct Player Player;

typedef struct Action {
    char command[32];
    int duration;
    time_t end_time;
    struct Action *next;
} Action;

void process_pending_action(Server *server);
void add_action_to_queue(Player *player, const char *command, int freq);
void handle_action_command(const char *command, char *response);
#endif