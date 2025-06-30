/*
** EPITECH PROJECT, 2025
** start
** File description:
** command.c
*/

#include "command/command.h"
#include "command/gui_commands.h"
#include "server.h"
#include "player.h"

int get_command_duration(const char *command)
{
    size_t len = 0;
    static commandduration_t durations[] = {{"Forward", 7}, {"Right", 7},
        {"Left", 7}, {"Look", 7}, {"Inventory", 1},
            {"Take", 7}, {"Set", 7}, {"Eject", 7},
                {"Broadcast", 7}, {"Incantation", 300},
                    {"Fork", 42}, {"Connect_nbr", 0}, {NULL, 0} };

    for (int i = 0; durations[i].name != NULL; i++) {
        len = strlen(durations[i].name);
        if (strncmp(command, durations[i].name, len) == 0 &&
            (command[len] == '\0' || command[len] == ' '))
            return durations[i].duration;
    }
    return -1;
}

static void handle_take_set_commands(player_t *player, server_t *server,
    const char *command, char *response)
{
    if (strncmp(command, "Take ", 5) == 0) {
        handle_take_command(player, server, command + 5, response);
        return;
    }
    if (strncmp(command, "Set ", 4) == 0) {
        handle_set_command(player, server, command + 4, response);
        return;
    }
    strcpy(response, "ko\n");
}

void handle_broadcast_command(player_t *player, server_t *server,
    const char *command, char *response)
{
    const char *text;
    int player_id;

    if (strncmp(command, "Broadcast ", 10) != 0) {
        strcpy(response, "ko\n");
        return;
    }
    text = command + 10;
    if (strlen(text) == 0) {
        strcpy(response, "ko\n");
        return;
    }
    player_id = player - server->players;
    handle_player_broadcast(server, player_id, text);
    strcpy(response, "ok\n");
}

static void handle_special_commands(player_t *player, server_t *server,
    const char *command, char *response)
{
    if (strcmp(command, "Fork") == 0) {
        handle_fork_command(player, server, response);
        return;
    }
    if (strcmp(command, "Eject") == 0) {
        handle_eject_command(player, server, response);
        return;
    }
    if (strcmp(command, "Incantation") == 0) {
        prepare_incantation(player, server, response);
        return;
    }
    if (strncmp(command, "Broadcast ", 10) == 0) {
        handle_broadcast_command(player, server, command, response);
        return;
    }
    strcpy(response, "ko\n");
}

static void handle_action_command(player_t *player, server_t *server,
    const char *command, char *response)
{
    if (strncmp(command, "Take ", 5) == 0 || strncmp(command, "Set ", 4) == 0){
        handle_take_set_commands(player, server, command, response);
        return;
    }
    handle_special_commands(player, server, command, response);
}

static void handle_info_command(player_t *player, server_t *server, const char
    *command, char *response)
{
    if (strcmp(command, "Look") == 0) {
        handle_look_command(player, server, response);
        return;
    }
    if (strcmp(command, "Inventory") == 0) {
        handle_inventory_command(player, response);
        return;
    }
    if (strcmp(command, "Connect_nbr") == 0) {
        snprintf(response, 1024, "%d\n",
        server->teams[player->team_id].max_clients -
            server->teams[player->team_id].current_clients);
    }
}

void handle_movement_command(player_t *player, server_t *server,
    const char *command, char *response)
{
    if (strcmp(command, "Forward") == 0) {
        move_player_forward(player, server);
        strcpy(response, "ok\n");
    }
    if (strcmp(command, "Right") == 0) {
        player->orientation = (player->orientation + 1) % 4;
        strcpy(response, "ok\n");
    }
    if (strcmp(command, "Left") == 0) {
        player->orientation = (player->orientation + 3) % 4;
        strcpy(response, "ok\n");
    }
}

void process_player_command(player_t *player, server_t *server,
    const char *command)
{
    char response[1024] = "";

    handle_movement_command(player, server, command, response);
    if (response[0] == '\0')
        handle_info_command(player, server, command, response);
    if (response[0] == '\0')
        handle_action_command(player, server, command, response);
    send(player->socket, response, strlen(response), 0);
}
