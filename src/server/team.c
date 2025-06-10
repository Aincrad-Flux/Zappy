/*
** EPITECH PROJECT, 2025
** start
** File description:
** team.c
*/

#include "../../include/server/team.h"
#include "../../include/server/server.h"

void add_team_name(Server *server, const char *name) {
    if (server->num_teams >= MAX_TEAMS) return;
    strncpy(server->teams[server->num_teams].name, name, MAX_TEAM_NAME - 1);
    server->teams[server->num_teams].name[MAX_TEAM_NAME - 1] = '\0';
    server->teams[server->num_teams].current_clients = 0;
    server->num_teams++;
}

void parse_team_names(Server *server, char *first_name, int argc, char **argv) {
    add_team_name(server, first_name);
    while (optind < argc && argv[optind][0] != '-') {
        add_team_name(server, argv[optind]);
        optind++;
    }
}

void set_team_max_clients(Server *server, int clients_nb) {
    for (int i = 0; i < server->num_teams; i++) {
        server->teams[i].max_clients = clients_nb;
    }
}

int find_team_by_name(Server *server, const char *team_name) {
    for (int i = 0; i < server->num_teams; i++) {
        if (strcmp(server->teams[i].name, team_name) == 0) {
            return i;
        }
    }
    return -1;
}

int can_join_team(Server *server, int team_id) {
    if (team_id == -1) return 0;
    return server->teams[team_id].current_clients < server->teams[team_id].max_clients;
}

void handle_team_join_success(Server *server, int client_socket, int team_id, const char *team_name) {
    if (server->num_players >= MAX_CLIENTS) {
        send(client_socket, "ko\n", 3, 0);
        return;
    }
    init_player(&server->players[server->num_players], client_socket, team_id, team_name, server);
    server->teams[team_id].current_clients++;
    send_connection_info(server, client_socket, team_id);
    server->num_players++;
    printf("Joueur connecté à l'équipe %s\n", team_name);
}

void handle_team_authentication(Server *server, int client_socket, const char *team_name) {
    int team_id = find_team_by_name(server, team_name);
    printf("→ ID %d (clients: %d/%d)\n", team_id,
           team_id >= 0 ? server->teams[team_id].current_clients : 0,
           team_id >= 0 ? server->teams[team_id].max_clients : 0);

    if (can_join_team(server, team_id)) {
        handle_team_join_success(server, client_socket, team_id, team_name);
        return;
    }

    send(client_socket, "ko\n", 3, 0);
    printf(team_id == -1 ? "Équipe inconnue\n" : "Équipe pleine\n");
}