/*
** EPITECH PROJECT, 2025
** start
** File description:
** server_main.c
*/

#include "server.h"
#include "time/tick.h"

void init_fd_sets(server_t *server)
{
    FD_ZERO(&server->master_fds);
    FD_SET(server->server_socket, &server->master_fds);
    server->max_fd = server->server_socket;
    server->num_players = 0;
}

void print_server_info(server_t *server)
{
    printf("Zappy server started on port %d\n", server->port);
    printf("World size: %dx%d\n", server->width, server->height);
    printf("Liste des équipes chargées :\n");
    for (int i = 0; i < server->num_teams; i++) {
        printf("  • %s (max_clients = %d)\n",
            server->teams[i].name, server->teams[i].max_clients);
    }
    printf("\n");
}

void run_server(server_t *server)
{
    fd_set read_fds;
    struct timeval timeout;
    int activity;

    while (1) {
        read_fds = server->master_fds;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        activity = select(server->max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (activity < 0)
            break;
        check_new_connections(server, &read_fds);
        check_client_messages(server, &read_fds);
        process_pending_action(server);
        update_ticks(server);
    }
}

int main(int argc, char **argv)
{
    server_t server;
    int parse_result;

    if (argc == 1) {
        print_usage(argv[0]);
        return 0;
    }
    parse_result = parse_arguments(argc, argv, &server);
    if (parse_result <= 0) {
        printf("ici\n");
        return parse_result == 0 ? 0 : 1;
    }
    srand(time(NULL));
    if (init_server(&server) < 0) {
        return 1;
    }
    run_server(&server);
    cleanup_server(&server);
    return 0;
}
