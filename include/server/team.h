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


typedef struct Server Server;
typedef struct {
    char name[MAX_TEAM_NAME];
    int max_clients;
    int current_clients;
} Team;

void add_team_name(Server *server, const char *name);
void parse_team_names(Server *server, char *first_name, int argc, char **argv);
void set_team_max_clients(Server *server, int clients_nb);
void handle_team_join_success(Server *server, int client_socket, int team_id, const char *team_name);
void handle_team_authentication(Server *server, int client_socket, const char *team_name);
#endif
