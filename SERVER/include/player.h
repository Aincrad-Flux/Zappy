/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** player
*/

#ifndef PLAYER
    #define PLAYER

    #include <string.h>
    #include <stdlib.h>
    #include <time.h>
    #include <string.h>
    #include "command/command.h"
    #include "utils/action.h"
    #include <stdbool.h>
    #define MAX_TEAM_NAME 50


typedef struct Server server_t;

typedef struct Player {
    int x, y;
    int orientation;
    int level;
    int *inventory;
    int team_id;
    int socket;
    char team_name[MAX_TEAM_NAME];
    time_t last_action;
    action_t *action_queue;
    bool is_incanting;
    bool is_waiting_level_up;
} player_t;

typedef struct player_init_s {
    int socket;
    int team_id;
    const char *team_name;
} player_init_t;

void init_player(player_t *player, player_init_t config, server_t *server);
void set_player_position(player_t *player, server_t *server);
void set_player_resources(player_t *player);
int find_player_by_socket(server_t *server, int socket);
void remove_player(server_t *server, int player_index);
void move_player_forward(player_t *player, server_t *server);

#endif
