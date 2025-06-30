/*
** EPITECH PROJECT, 2025
** zappy
** File description:
** kill_player
*/

#include "utils/kill_player.h"
#include "player.h"
#include "server.h"
#include "command/gui_commands.h"

static void cleanup_player_resources(player_t *player)
{
    if (player->inventory != NULL) {
        free(player->inventory);
        player->inventory = NULL;
    }
}

static void cleanup_player_actions(player_t *player)
{
    action_t *current_action = player->action_queue;
    
    while (current_action != NULL) {
        action_t *next = current_action->next;
        free(current_action);
        current_action = next;
    }
    player->action_queue = NULL;
}

static void update_max_fd(server_t *server, int excluded_index)
{
    server->max_fd = 0;
    for (int i = 0; i < server->num_players; i++)
        if (i != excluded_index && server->players[i].socket > server->max_fd)
            server->max_fd = server->players[i].socket;
    if (server->server_socket > server->max_fd)
        server->max_fd = server->server_socket;
}

static void cleanup_player_socket(server_t *server, player_t *player, int player_index)
{
    if (player->socket == -1)
        return;
    close(player->socket);
    FD_CLR(player->socket, &server->master_fds);
    if (player->socket == server->max_fd)
        update_max_fd(server, player_index);
}

static void shift_players_array(server_t *server, int player_index)
{
    for (int i = player_index; i < server->num_players - 1; i++)
        server->players[i] = server->players[i + 1];
    memset(&server->players[server->num_players - 1], 0, sizeof(player_t));
    server->num_players--;
}

void kill_player(server_t *server, int player_index)
{
    player_t *player;

    if (player_index < 0 || player_index >= server->num_players)
        return;
    player = &server->players[player_index];
    cleanup_player_resources(player);
    cleanup_player_actions(player);
    cleanup_player_socket(server, player, player_index);
    shift_players_array(server, player_index);
    send_gui_pdi(server, player_index);
}