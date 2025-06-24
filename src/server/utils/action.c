/*
** EPITECH PROJECT, 2025
** start
** File description:
** action.c
*/

#include "../../../include/server/utils/action.h"
#include "../../../include/server/player.h"
#include "../../../include/server/server.h"
#include "../../../include/server/command/command.h"

static void add_action(Player *player, time_t base_time, Action *new_action,
    int freq, const char *command)
{
    Action *curr;
    int duration_ticks = get_command_duration(command);

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

void add_action_to_queue(Player *player, const char *command, int freq)
{
    Action *new_action = malloc(sizeof(Action));
    time_t base_time;
    int duration_ticks = get_command_duration(command);

    if (!new_action)
        return;
    base_time = time(NULL);
    strncpy(new_action->command, command, sizeof(new_action->command) - 1);
    new_action->command[sizeof(new_action->command) - 1] = '\0';
    add_action(player, base_time, new_action, freq, command);
    new_action->duration = duration_ticks;
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
    for (int i = 0; i < server->num_players; i++)
        handle_action(server, i);
}
