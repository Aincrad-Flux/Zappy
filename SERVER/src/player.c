/*
** EPITECH PROJECT, 2025
** start
** File description:
** player.c
*/

#include "player.h"
#include "server.h"
#include "map/resource.h"
#include "command/gui_commands.h"

static void set_player_position(player_t *player, server_t *server)
{
    tile_t *tile;

    player->x = rand() % server->width;
    player->y = rand() % server->height;
    player->orientation = rand() % 4;
    player->level = 1;
    player->action_queue = NULL;
    tile = get_tile(server->map, player->x, player->y);
    add_player_to_tile(tile, player);
}

void set_player_resources(player_t *player)
{
    if (!player->inventory) {
        player->inventory = malloc(sizeof(int) * RESOURCE_COUNT);
    }
    for (int i = 0; i < RESOURCE_COUNT; i++) {
        player->inventory[i] = 0;
        if (i == 0)
            player->inventory[i] = 10;
    }
    player->last_action = time(NULL);
}

void init_player(player_t *player, player_init_t config, server_t *server)
{
    player->inventory = NULL;
    player->socket = config.socket;
    player->team_id = config.team_id;
    strncpy(player->team_name, config.team_name, MAX_TEAM_NAME - 1);
    player->team_name[MAX_TEAM_NAME - 1] = '\0';
    set_player_position(player, server);
    set_player_resources(player);
}

int find_player_by_socket(server_t *server, int socket)
{
    for (int i = 0; i < server->num_players; i++) {
        if (server->players[i].socket == socket) {
            return i;
        }
    }
    return -1;
}

void remove_player(server_t *server, int player_index)
{
    if (server->players[player_index].inventory) {
        free(server->players[player_index].inventory);
    }
    server->teams[server->players[player_index].team_id].current_clients--;
    for (int i = player_index; i < server->num_players - 1; i++) {
        server->players[i] = server->players[i + 1];
    }
    server->num_players--;
}

void send_player_info(server_t *serv, int graphic_fd)
{
    player_t *player;
    char buffer[256];

    for (int i = 0; i < serv->num_players; i++) {
        player = &serv->players[i];
        snprintf(buffer, sizeof(buffer), "pnw #%d %d %d %d %d %s\n", i,
                player->x, player->y, player->orientation + 1, player->level,
                serv->teams[player->team_id].name);
        send(graphic_fd, buffer, strlen(buffer), 0);
        send_gui_ppo(server, player->socket);
        snprintf(buffer, sizeof(buffer), "plv #%d %d\n", i, player->level);
        send(graphic_fd, buffer, strlen(buffer), 0);
        snprintf(buffer, sizeof(buffer), "pin #%d %d %d %d %d %d %d %d "
                "%d %d\n", i, player->x, player->y, player->inventory[FOOD],
                player->inventory[LINEMATE], player->inventory[DERAUMERE],
                player->inventory[SIBUR], player->inventory[MENDIANE],
                player->inventory[PHIRAS], player->inventory[THYSTAME]);
        send(graphic_fd, buffer, strlen(buffer), 0);
    }
}
