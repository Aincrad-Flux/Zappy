/*
** EPITECH PROJECT, 2024
**
** File description:
** struct player
*/

#ifndef PLAYER
    #define PLAYER

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "utils/command.h"
#include "utils/action.h"

#define MAX_TEAM_NAME 50


typedef struct Server Server;

typedef struct Player {
    int x, y;
    int orientation;
    int level;
    int food;
    int linemate, deraumere, sibur, mendiane, phiras, thystame;
    int team_id;
    int socket;
    char team_name[MAX_TEAM_NAME];
    time_t last_action;
    Action *action_queue;
} Player;

void init_player(Player *player, int socket, int team_id, const char *team_name, Server *server);
void set_player_position(Player *player, Server *server);
void set_player_resources(Player *player);
int find_player_by_socket(Server *server, int socket);
void remove_player(Server *server, int player_index);
void move_player_forward(Player *player, Server *server);

#endif