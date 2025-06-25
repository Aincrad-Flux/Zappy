/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** server_init
*/

#include "server.h"
#include "time/tick.h"

void print_usage(char *program_name)
{
    printf("USAGE: %s -p port -x width -y height -n name1 ", program_name);
    printf("name2 ... -c clientsNb -f freq\n");
    printf("  -p port      : port number\n");
    printf("  -x width     : world width\n");
    printf("  -y height    : world height\n");
    printf("  -n names     : team names\n");
    printf("  -c clientsNb : number of authorized clients per team ");
    printf("at the beginning\n");
    printf("  -f freq      : reciprocal of time unit ");
    printf("for execution of actions\n");
}

static void init_server_defaults(server_t *server)
{
    server->port = 4242;
    server->width = 10;
    server->height = 10;
    server->freq = 100;
    server->num_teams = 0;
}

static int handle_parse_port(server_t *server, char *optarg)
{
    server->port = atoi(optarg);
    return 0;
}

static int handle_parse_dimensions(server_t *server, char *optarg, char opt)
{
    if (opt == 'x')
        server->width = atoi(optarg);
    if (opt == 'y')
        server->height = atoi(optarg);
    return 0;
}

static int handle_parse_teams(server_t *server, char *optarg,
    char **argv)
{
    parse_team_names(server, optarg, argv);
    return 0;
}

static int handle_parse_option(server_t *server, int opt, char *optarg,
    char **argv)
{
    if (opt == 'p')
        return handle_parse_port(server, optarg);
    if (opt == 'x' || opt == 'y')
        return handle_parse_dimensions(server, optarg, opt);
    if (opt == 'n')
        return handle_parse_teams(server, optarg, argv);
    if (opt == 'c')
        return atoi(optarg);
    if (opt == 'f') {
        server->freq = atoi(optarg);
        return 0;
    }
    if (opt == 'h') {
        print_usage(argv[0]);
        return -2;
    }
    print_usage(argv[0]);
    return -1;
}

int parse_arguments(int argc, char **argv, server_t *server)
{
    int opt;
    int clients_nb = 0;
    int result;

    init_server_defaults(server);
    opt = getopt(argc, argv, "p:x:y:n:c:f:h");
    while (opt != -1) {
        result = handle_parse_option(server, opt, optarg, argv);
        if (result == -2)
            return 0;
        if (result == -1)
            return -1;
        if (result > 0)
            clients_nb = result;
        opt = getopt(argc, argv, "p:x:y:n:c:f:h");
    }
    set_team_max_clients(server, clients_nb);
    if (server->num_teams > 0)
        return 1;
    return -1;
}

static int create_server_socket(server_t *server)
{
    int opt = 1;

    server->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_socket < 0)
        return -1;
    if (setsockopt(server->server_socket, SOL_SOCKET,
        SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        return -1;
    }
    return 0;
}

static int bind_server_socket(server_t *server)
{
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server->port);
    if (bind(server->server_socket, (struct sockaddr*)&server_addr,
            sizeof(server_addr)) < 0) {
        return -1;
    }
    return listen(server->server_socket, MAX_CLIENTS);
}

int init_server(server_t *server)
{
    if (create_server_socket(server) < 0)
        return -1;
    if (bind_server_socket(server) < 0)
        return -1;
    init_fd_sets(server);
    print_server_info(server);
    init_map(server);
    return 0;
}
