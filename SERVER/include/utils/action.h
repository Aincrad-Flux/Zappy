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
    #define BUFFER_SIZE 1024

typedef struct Server server_t;
typedef struct Player player_t;

typedef struct Action {
    char command[32];
    int duration;
    time_t end_time;
    struct Action *next;
} action_t;

void process_pending_action(server_t *server);
void add_action_to_queue(player_t *player, const char *command, int freq);
void kill_player(server_t *server, int player_index);
#endif
