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
    team_t teams[MAX_TEAMS];
    int num_teams;
    player_t players[MAX_CLIENTS];
    int num_players;
    int server_socket;
    fd_set master_fds;
    int max_fd;
    time_t last_tick;
    int tick_count;
    int graphic_fd;
    map_t *map;
    int next_egg_id;
} server_t;

int parse_arguments(int argc, char **argv, server_t *server);
void init_fd_sets(server_t *server);
int init_server(server_t *server);
void check_new_connections(server_t *server, fd_set *read_fds);
void check_client_messages(server_t *server, fd_set *read_fds);
void send_connection_info(server_t *server, int client_socket, int team_id);
void cleanup_server(server_t *server);
void run_server(server_t *server);
void print_usage(char *program_name);
void print_server_info(server_t *server);

#endif
