/*
** EPITECH PROJECT, 2025
** start
** File description:
** team.h
*/

#ifndef TEAM
    #define TEAM
    #define MAX_TEAM_NAME 50
    #include <stdio.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/socket.h>


typedef struct Server server_t;
typedef struct Team {
    char name[MAX_TEAM_NAME];
    int max_clients;
    int current_clients;
} team_t;

void add_team_name(server_t *server, const char *name);
void parse_team_names(server_t *server, char *first_name, char **argv);
void set_team_max_clients(server_t *server, int clients_nb);
void handle_team_join_success(server_t *server, int client_socket, int team_id,
    const char *team_name);
void handle_team_authentication(server_t *server, int client_socket, const char
    *team_name);
#endif
