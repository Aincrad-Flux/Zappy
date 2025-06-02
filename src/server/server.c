/*
** EPITECH PROJECT, 2025
** start
** File description:
** server.c
*/

#include "../../include/server/server.h"
#define BUFFER_SIZE 1024

void print_usage(char *program_name) {
    printf("USAGE: %s -p port -x width -y height -n name1 name2 ... -c clientsNb -f freq\n", program_name);
    printf("  -p port      : port number\n");
    printf("  -x width     : world width\n");
    printf("  -y height    : world height\n");
    printf("  -n names     : team names\n");
    printf("  -c clientsNb : number of authorized clients per team at the beginning\n");
    printf("  -f freq      : reciprocal of time unit for execution of actions\n");
}

void init_server_defaults(Server *server) {
    server->port = 4242;
    server->width = 10;
    server->height = 10;
    server->freq = 100;
    server->num_teams = 0;
}

int handle_parse_option(Server *server, int opt, char *optarg, int argc, char **argv) {
    switch (opt) {
        case 'p': server->port = atoi(optarg); break;
        case 'x': server->width = atoi(optarg); break;
        case 'y': server->height = atoi(optarg); break;
        case 'n': parse_team_names(server, optarg, argc, argv); break;
        case 'c': return atoi(optarg);
        case 'f': server->freq = atoi(optarg); break;
        case 'h': print_usage(argv[0]); return -2;
        default: print_usage(argv[0]); return -1;
    }
    return 0;
}

int parse_arguments(int argc, char **argv, Server *server) {
    int opt, clients_nb = 0, result;

    init_server_defaults(server);
    while ((opt = getopt(argc, argv, "p:x:y:n:c:f:h")) != -1) {
        result = handle_parse_option(server, opt, optarg, argc, argv);
        if (result == -2) return 0;
        if (result == -1) return -1;
        if (result > 0) clients_nb = result;
    }
    set_team_max_clients(server, clients_nb);
    return server->num_teams > 0 ? 1 : -1;
}

int create_server_socket(Server *server) {
    int opt = 1;
    server->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_socket < 0) return -1;

    if (setsockopt(server->server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        return -1;
    }
    return 0;
}

int bind_server_socket(Server *server) {
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server->port);

    if (bind(server->server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        return -1;
    }
    return listen(server->server_socket, MAX_CLIENTS);
}

void init_fd_sets(Server *server) {
    FD_ZERO(&server->master_fds);
    FD_SET(server->server_socket, &server->master_fds);
    server->max_fd = server->server_socket;
    server->num_players = 0;
}

void print_server_info(Server *server) {
    printf("Zappy server started on port %d\n", server->port);
    printf("World size: %dx%d\n", server->width, server->height);
    printf("Liste des équipes chargées :\n");
    for (int i = 0; i < server->num_teams; i++) {
        printf("  • %s (max_clients = %d)\n", server->teams[i].name, server->teams[i].max_clients);
    }
    printf("\n");
}

int init_server(Server *server) {
    if (create_server_socket(server) < 0) return -1;
    if (bind_server_socket(server) < 0) return -1;

    init_fd_sets(server);
    print_server_info(server);
    return 0;
}

void add_client_to_fds(Server *server, int new_socket) {
    FD_SET(new_socket, &server->master_fds);
    if (new_socket > server->max_fd) {
        server->max_fd = new_socket;
    }
}

void handle_new_connection(Server *server) {
    int new_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    new_socket = accept(server->server_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (new_socket < 0) return;

    printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    snprintf(buffer, BUFFER_SIZE, "WELCOME\n");
    send(new_socket, buffer, strlen(buffer), 0);
    add_client_to_fds(server, new_socket);
}

void handle_client_disconnect(Server *server, int client_socket) {
    int player_index = find_player_by_socket(server, client_socket);

    printf("Client disconnected\n");
    close(client_socket);
    FD_CLR(client_socket, &server->master_fds);

    if (player_index != -1) {
        remove_player(server, player_index);
    }
}

void clean_message_buffer(char *buffer, int bytes_received) {
    for (int i = 0; i < bytes_received; i++) {
        if (buffer[i] == '\n' || buffer[i] == '\r') {
            buffer[i] = '\0';
            break;
        }
    }
}

void send_connection_info(Server *server, int client_socket, int team_id) {
    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "%d\n%d %d\n",
             server->teams[team_id].max_clients - server->teams[team_id].current_clients,
             server->width, server->height);
    send(client_socket, response, strlen(response), 0);
}

void handle_client_message(Server *server, int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    int player_index = find_player_by_socket(server, client_socket);

    if (bytes_received <= 0) {
        handle_client_disconnect(server, client_socket);
        return;
    }

    buffer[bytes_received] = '\0';
    clean_message_buffer(buffer, bytes_received);
    printf("Tentative de connexion avec : '%s'\n", buffer);
    printf("%d\n", player_index);
    if (player_index == -1) {
        handle_team_authentication(server, client_socket, buffer);
    } else {
        add_action_to_queue(&server->players[player_index], buffer, server->freq);
    }
}

void check_new_connections(Server *server, fd_set *read_fds) {
    if (FD_ISSET(server->server_socket, read_fds)) {
        handle_new_connection(server);
    }
}

void check_client_messages(Server *server, fd_set *read_fds) {
    for (int i = 0; i <= server->max_fd; i++) {
        if (i != server->server_socket && FD_ISSET(i, read_fds)) {
            handle_client_message(server, i);
        }
    }
}

void run_server(Server *server) {
    fd_set read_fds;
    struct timeval timeout;
    int activity;

    while (1) {
        read_fds = server->master_fds;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        activity = select(server->max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (activity < 0) break;

        check_new_connections(server, &read_fds);
        check_client_messages(server, &read_fds);
        process_pending_action(server);
    }
}

void cleanup_server(Server *server) {
    for (int i = 0; i < server->num_players; i++) {
        close(server->players[i].socket);
    }
    close(server->server_socket);
}

int main(int argc, char **argv) {
    Server server;
    int parse_result;

    if (argc == 1) {
        print_usage(argv[0]);
        return 0;
    }

    parse_result = parse_arguments(argc, argv, &server);
    if (parse_result <= 0) return parse_result == 0 ? 0 : 1;

    srand(time(NULL));
    if (init_server(&server) < 0) return 1;

    run_server(&server);
    cleanup_server(&server);
    return 0;
}