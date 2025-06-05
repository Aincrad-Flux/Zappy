/*
** EPITECH PROJECT, 2025
** start
** File description:
** action.c
*/

#include "../../../include/server/utils/action.h"
#include "../../../include/server/player.h"
#include "../../../include/server/server.h"
#define BUFFER_SIZE 1024

void add_action_to_queue(Player *player, const char *command, int freq)
{
    Action *new_action = malloc(sizeof(Action));
    time_t base_time;
    int duration_ticks = get_command_duration(command);
    Action *curr;

    if (!new_action)
        return;
    base_time = time(NULL);
    strncpy(new_action->command, command, sizeof(new_action->command) - 1);
    new_action->command[sizeof(new_action->command) - 1] = '\0';
    if (player->action_queue == NULL) {
        new_action->end_time = base_time +
                (int)ceil((double)duration_ticks / freq);
        new_action->next = NULL;
        player->action_queue = new_action;
    } else {
        curr = player->action_queue;
        while (curr->next)
            curr = curr->next;
        new_action->end_time = curr->end_time +
                (int)ceil((double)duration_ticks / freq);
        new_action->next = NULL;
        curr->next = new_action;
    }
    new_action->duration = duration_ticks;
}

static void handle_action_command(const char *command, char *response)
{
    if (strcmp(command, "Broadcast") == 0 || strcmp(command, "Fork") == 0 ||
        strcmp(command, "Eject") == 0 || strncmp(command, "Take ", 5) == 0 ||
        strncmp(command, "Set ", 4) == 0) {
        strcpy(response, "ok\n");
    } else if (strcmp(command, "Incantation") == 0) {
        strcpy(response, "ko\n");
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
        snprintf(response, BUFFER_SIZE, "[ food %d, linemate %d, deraumere %d, sibur %d, mendiane %d, phiras %d, thystame %d ]\n",
                 player->food, player->linemate, player->deraumere, player->sibur,
                 player->mendiane, player->phiras, player->thystame);
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
        handle_action_command(command, response);
    send(player->socket, response, strlen(response), 0);
}

void next_action(Action **action_queue)
{
    Action *to_remove = NULL;

    if (!action_queue || !*action_queue)
        return;
    to_remove = *action_queue;
    *action_queue = to_remove->next;
    free(to_remove);
}

static void handle_action(Server *server, int i)
{
    Player *player = &server->players[i];
    Action *current_action = NULL;
    time_t now;

    if (player->action_queue == NULL)
        return;
    current_action = player->action_queue;
    now = time(NULL);
    if (now >= current_action->end_time) {
        process_player_command(player, server, current_action->command);
        next_action(&player->action_queue);
    }
}

void process_pending_action(Server *server)
{
    for (int i = 0; i < server->num_players; i++) {
        handle_action(server, i);
    }
}