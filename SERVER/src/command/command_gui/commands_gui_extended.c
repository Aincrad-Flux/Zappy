/*
** EPITECH PROJECT, 2025
** zappy
** File description:
** gui_commands_extended.c
*/

#include "server.h"
#include "command/gui_commands.h"
#include "map/resource.h"

void handle_gui_sst(server_t *server, int client_socket, int new_freq)
{
    char buffer[256];

    if (new_freq <= 0) {
        send(client_socket, "sbp\n", 4, 0);
        return;
    }
    server->freq = new_freq;
    snprintf(buffer, sizeof(buffer), "sst %d\n", server->freq);
    send(client_socket, buffer, strlen(buffer), 0);
}

void send_gui_pnw(server_t *server, int player_id)
{
    char buffer[256];
    player_t *player = &server->players[player_id];

    snprintf(buffer, sizeof(buffer), "pnw #%d %d %d %d %d %s\n",
        player_id, player->x, player->y, player->orientation + 1,
            player->level, server->teams[player->team_id].name);
    broadcast_to_gui_clients(server, buffer);
}

void send_gui_pex(server_t *server, int player_id)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "pex #%d\n", player_id);
    broadcast_to_gui_clients(server, buffer);
}

void send_gui_pbc(server_t *server, int player_id, const char *message)
{
    char buffer[512];

    snprintf(buffer, sizeof(buffer), "pbc #%d %s\n", player_id, message);
    broadcast_to_gui_clients(server, buffer);
}

static void build_pic_buffer(char *buffer, player_t *player,
    list_t *players_on_tile)
{
    char temp[32];
    list_t *current = players_on_tile;

    snprintf(buffer, 512, "pic %d %d %d", player->x, player->y, player->level);
    while (current != NULL) {
        snprintf(temp, sizeof(temp), " #%d", current->player->team_id);
        strcat(buffer, temp);
        current = current->next;
    }
    strcat(buffer, "\n");
}

void send_gui_pic(server_t *server, player_t *player)
{
    char buffer[512];
    tile_t *tile = &server->map->tiles[player->y][player->x];

    build_pic_buffer(buffer, player, tile->players_on_tile);
    broadcast_to_gui_clients(server, buffer);
}
