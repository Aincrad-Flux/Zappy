/*
** EPITECH PROJECT, 2025
** start
** File description:
** command.c
*/

#include "../../../include/server/command/command.h"
#include "../../../include/server/server.h"
#include "../../../include/server/player.h"

#define BUFFER_SIZE 1024


int get_command_duration(const char *command)
{
    size_t len = 0;
    static CommandDuration durations[] = {{"Forward", 7}, {"Right", 7},
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

static void handle_action_command(Player *player, Server *server, const char
*command, char *response)
{
    if (strncmp(command, "Take ", 5) == 0) {
        handle_take_command(player, server, command + 5, response);
    } else if (strncmp(command, "Set ", 4) == 0) {
        handle_set_command(player, server, command + 4, response);
    } else if (strncmp(command, "Broadcast ", 10) == 0) {
        handle_broadcast_command(player, server, command + 10, response);
    } else if (strcmp(command, "Fork") == 0) {
        handle_fork_command(player, server, response);
    } else if (strcmp(command, "Eject") == 0) {
        handle_eject_command(player, server, response);
    } else if (strcmp(command, "Incantation") == 0) {
        handle_incantation_command(player, server, response);
    } else {
        strcpy(response, "ko\n");
    }
}

static void handle_info_command(Player *player, Server *server, const char
*command, char *response)
{
    if (strcmp(command, "Look") == 0) {
        strcpy(response, "[ player ]\n");
    } else if (strcmp(command, "Inventory") == 0) {
        handle_inventory_command(player, response);
    } else if (strcmp(command, "Connect_nbr") == 0) {
        snprintf(response, BUFFER_SIZE, "%d\n",
                 server->teams[player->team_id].max_clients -
                 server->teams[player->team_id].current_clients);
    }
}

void handle_movement_command(Player *player, Server *server,
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

void process_player_command(Player *player, Server *server,
                            const char *command)
{
    char response[BUFFER_SIZE] = "";

    handle_movement_command(player, server, command, response);
    if (response[0] == '\0')
        handle_info_command(player, server, command, response);
    if (response[0] == '\0')
        handle_action_command(player, server, command, response);
    send(player->socket, response, strlen(response), 0);
}
