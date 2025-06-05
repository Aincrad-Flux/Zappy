/*
** EPITECH PROJECT, 2025
** start
** File description:
** team.h
*/

#ifndef SERVER_H
#define SERVER_H

#define MAX_TEAMS 10
#define MAX_CLIENTS 100
#define MAX_TEAM_NAME 50

#include "player.h"
#include "team.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>
#include "utils/action.h"
#include "map/map.h"

typedef struct Server {
    int width, height;
    int port;
    int freq;
    Team teams[MAX_TEAMS];
    int num_teams;
    Player players[MAX_CLIENTS];
    int num_players;
    int server_socket;
    fd_set master_fds;
    int max_fd;
    time_t last_tick;
    int tick_count;
    Map *map;
} Server;

void send_connection_info(Server *server, int client_socket, int team_id);

#endif
