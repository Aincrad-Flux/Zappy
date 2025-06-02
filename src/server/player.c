/*
** EPITECH PROJECT, 2025
** start
** File description:
** player.c
*/


#include "../../include/server/player.h"
#include "../../include/server/server.h"

void set_player_position(Player *player, Server *server) {
    player->x = rand() % server->width;
    player->y = rand() % server->height;
    player->orientation = rand() % 4;
    player->level = 1;
    player->food = 10;
    player->action_queue = NULL;
}

void set_player_resources(Player *player) {
    player->linemate = 0;
    player->deraumere = 0;
    player->sibur = 0;
    player->mendiane = 0;
    player->phiras = 0;
    player->thystame = 0;
    player->last_action = time(NULL);
}

void init_player(Player *player, int socket, int team_id, const char *team_name, Server *server) {
    player->socket = socket;
    player->team_id = team_id;
    strncpy(player->team_name, team_name, MAX_TEAM_NAME - 1);
    player->team_name[MAX_TEAM_NAME - 1] = '\0';

    set_player_position(player, server);
    set_player_resources(player);
}

int find_player_by_socket(Server *server, int socket) {
    for (int i = 0; i < server->num_players; i++) {
        if (server->players[i].socket == socket) {
            return i;
        }
    }
    return -1;
}

void remove_player(Server *server, int player_index) {
    server->teams[server->players[player_index].team_id].current_clients--;
    for (int i = player_index; i < server->num_players - 1; i++) {
        server->players[i] = server->players[i + 1];
    }
    server->num_players--;
}

void move_player_forward(Player *player, Server *server) {
    switch (player->orientation) {
        case 0: player->y = (player->y - 1 + server->height) % server->height; break;
        case 1: player->x = (player->x + 1) % server->width; break;
        case 2: player->y = (player->y + 1) % server->height; break;
        case 3: player->x = (player->x - 1 + server->width) % server->width; break;
    }
}
