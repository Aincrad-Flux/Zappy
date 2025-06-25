/*
** EPITECH PROJECT, 2025
** zappy
** File description:
** gui_command_processor.c
*/

#include "server.h"
#include "command/gui_commands.h"

static void process_basic_commands(server_t *server, int client_socket,
    char *cmd)
{
    if (strcmp(cmd, "msz") == 0) {
        handle_gui_msz(server, client_socket);
        return;
    }
    if (strcmp(cmd, "mct") == 0) {
        handle_gui_mct(server, client_socket);
        return;
    }
    if (strcmp(cmd, "tna") == 0) {
        handle_gui_tna(server, client_socket);
        return;
    }
    send(client_socket, "suc\n", 4, 0);
}

static void process_coordinate_commands(server_t *server, int client_socket,
    char *cmd)
{
    char *arg1 = strtok(NULL, " ");
    char *arg2 = strtok(NULL, " ");

    if (strcmp(cmd, "bct") == 0) {
        if (arg1 && arg2) {
            handle_gui_bct(server, client_socket, atoi(arg1), atoi(arg2));
        } else {
            send(client_socket, "sbp\n", 4, 0);
        }
        return;
    }
    send(client_socket, "suc\n", 4, 0);
}

static void process_player_commands(server_t *server, int client_socket,
    char *cmd)
{
    char *arg1 = strtok(NULL, " ");

    if (!arg1 || arg1[0] != '#') {
        send(client_socket, "sbp\n", 4, 0);
        return;
    }
    if (strcmp(cmd, "ppo") == 0) {
        handle_gui_ppo(server, client_socket, atoi(arg1 + 1));
        return;
    }
    if (strcmp(cmd, "plv") == 0) {
        handle_gui_plv(server, client_socket, atoi(arg1 + 1));
        return;
    }
    if (strcmp(cmd, "pin") == 0) {
        handle_gui_pin(server, client_socket, atoi(arg1 + 1));
        return;
    }
    send(client_socket, "suc\n", 4, 0);
}

static void process_server_commands(server_t *server, int client_socket,
    char *cmd)
{
    char *arg1;

    if (strcmp(cmd, "sgt") == 0) {
        handle_gui_sgt(server, client_socket);
        return;
    }
    if (strcmp(cmd, "sst") == 0) {
        arg1 = strtok(NULL, " ");
        if (arg1) {
            handle_gui_sst(server, client_socket, atoi(arg1));
        } else {
            send(client_socket, "sbp\n", 4, 0);
        }
        return;
    }
    send(client_socket, "suc\n", 4, 0);
}

static int is_basic_command(char *cmd)
{
    return (strcmp(cmd, "msz") == 0 || strcmp(cmd, "mct") == 0 ||
        strcmp(cmd, "tna") == 0);
}

static int is_player_command(char *cmd)
{
    return (strcmp(cmd, "ppo") == 0 || strcmp(cmd, "plv") == 0 ||
        strcmp(cmd, "pin") == 0);
}

static void route_special_commands(server_t *server, int client_socket,
    char *cmd)
{
    if (strcmp(cmd, "bct") == 0) {
        process_coordinate_commands(server, client_socket, cmd);
    } else if (is_player_command(cmd)) {
        process_player_commands(server, client_socket, cmd);
    } else {
        process_server_commands(server, client_socket, cmd);
    }
}

static void route_command(server_t *server, int client_socket, char *cmd)
{
    if (is_basic_command(cmd)) {
        process_basic_commands(server, client_socket, cmd);
    } else {
        route_special_commands(server, client_socket, cmd);
    }
}

void process_gui_command(server_t *server, int client_socket, char *command)
{
    char *cmd = strtok(command, " \n");

    if (!cmd) {
        send(client_socket, "suc\n", 4, 0);
        return;
    }
    route_command(server, client_socket, cmd);
}
