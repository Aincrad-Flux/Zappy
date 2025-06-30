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

static void build_pic_buffer(char *buffer, gui_pic_data_t *data)
{
    char temp[32];

    snprintf(buffer, 512, "pic %d %d %d", data->x, data->y, data->level);
    for (int i = 0; i < data->nb_players; i++) {
        snprintf(temp, sizeof(temp), " #%d", data->players[i]);
        strcat(buffer, temp);
    }
    strcat(buffer, "\n");
}

void send_gui_pic(server_t *server, gui_pic_data_t *data)
{
    char buffer[512];

    build_pic_buffer(buffer, data);
    broadcast_to_gui_clients(server, buffer);
}
