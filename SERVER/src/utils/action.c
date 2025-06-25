/*
** EPITECH PROJECT, 2025
** start
** File description:
** action.c
*/

#include "utils/action.h"
#include "player.h"
#include "server.h"
#include "command/command.h"

static void add_action(player_t *player, time_t base_time, action_t *new_action,
    int freq)
{
    action_t *curr;
    int duration_ticks = get_command_duration(curr->command);

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
}

void add_action_to_queue(player_t *player, const char *command, int freq)
{
    action_t *new_action = malloc(sizeof(action_t));
    time_t base_time;
    int duration_ticks = get_command_duration(command);

    if (!new_action)
        return;
    base_time = time(NULL);
    strncpy(new_action->command, command, sizeof(new_action->command) - 1);
    new_action->command[sizeof(new_action->command) - 1] = '\0';
    add_action(player, base_time, new_action, freq);
    new_action->duration = duration_ticks;
}

void next_action(action_t **action_queue)
{
    action_t *to_remove = NULL;

    if (!action_queue || !*action_queue)
        return;
    to_remove = *action_queue;
    *action_queue = to_remove->next;
    free(to_remove);
}

static void handle_action(server_t *server, int i)
{
    player_t *player = &server->players[i];
    action_t *current_action = NULL;
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

void process_pending_action(server_t *server)
{
    for (int i = 0; i < server->num_players; i++)
        handle_action(server, i);
}
