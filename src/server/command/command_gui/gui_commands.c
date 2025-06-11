/*
** EPITECH PROJECT, 2025
** zappy
** File description:
** gui_commands.c
*/

#include "../../../../include/server/server.h"
#include "../../../../include/server/command/gui_commands.h"

void handle_gui_msz(Server *server, int client_socket)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "msz %d %d\n",
             server->width, server->height);
    send(client_socket, buffer, strlen(buffer), 0);
}

void handle_gui_bct(Server *server, int client_socket, int x, int y)
{
    char buffer[256];
    Tile *tile;

    if (x < 0 || x >= server->width || y < 0 || y >= server->height) {
        send(client_socket, "sbp\n", 4, 0);
        return;
    }
    tile = &server->map->tiles[y][x];
    snprintf(buffer, sizeof(buffer), "bct %d %d %d %d %d %d %d %d %d\n",
             x, y, tile->food, tile->linemate, tile->deraumere,
             tile->sibur, tile->mendiane, tile->phiras, tile->thystame);
    send(client_socket, buffer, strlen(buffer), 0);
}

void handle_gui_mct(Server *server, int client_socket)
{
    for (int y = 0; y < server->height; y++) {
        for (int x = 0; x < server->width; x++) {
            handle_gui_bct(server, client_socket, x, y);
        }
    }
}

void handle_gui_tna(Server *server, int client_socket)
{
    char buffer[256];

    for (int i = 0; i < server->num_teams; i++) {
        snprintf(buffer, sizeof(buffer), "tna %s\n",
                 server->teams[i].name);
        send(client_socket, buffer, strlen(buffer), 0);
    }
}

void handle_gui_ppo(Server *server, int client_socket, int player_id)
{
    char buffer[256];
    Player *player;

    if (player_id < 0 || player_id >= server->num_players) {
        send(client_socket, "sbp\n", 4, 0);
        return;
    }
    player = &server->players[player_id];
    snprintf(buffer, sizeof(buffer), "ppo #%d %d %d %d\n",
             player_id, player->x, player->y, player->orientation + 1);
    send(client_socket, buffer, strlen(buffer), 0);
}

void handle_gui_plv(Server *server, int client_socket, int player_id)
{
    char buffer[256];
    Player *player;

    if (player_id < 0 || player_id >= server->num_players) {
        send(client_socket, "sbp\n", 4, 0);
        return;
    }
    player = &server->players[player_id];
    snprintf(buffer, sizeof(buffer), "plv #%d %d\n",
             player_id, player->level);
    send(client_socket, buffer, strlen(buffer), 0);
}

void handle_gui_pin(Server *server, int client_socket, int player_id)
{
    char buffer[256];
    Player *player;

    if (player_id < 0 || player_id >= server->num_players) {
        send(client_socket, "sbp\n", 4, 0);
        return;
    }
    player = &server->players[player_id];
    snprintf(buffer, sizeof(buffer),
             "pin #%d %d %d %d %d %d %d %d %d %d\n",
             player_id, player->x, player->y, player->food,
             player->linemate, player->deraumere, player->sibur,
             player->mendiane, player->phiras, player->thystame);
    send(client_socket, buffer, strlen(buffer), 0);
}

void handle_gui_sgt(Server *server, int client_socket)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "sgt %d\n", server->freq);
    send(client_socket, buffer, strlen(buffer), 0);
}

void handle_gui_sst(Server *server, int client_socket, int new_freq)
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

void send_gui_pnw(Server *server, int player_id)
{
    char buffer[256];
    Player *player = &server->players[player_id];

    snprintf(buffer, sizeof(buffer), "pnw #%d %d %d %d %d %s\n",
             player_id, player->x, player->y, player->orientation + 1,
             player->level, server->teams[player->team_id].name);
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_pex(Server *server, int player_id)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "pex #%d\n", player_id);
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_pbc(Server *server, int player_id, const char *message)
{
    char buffer[512];

    snprintf(buffer, sizeof(buffer), "pbc #%d %s\n", player_id, message);
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_pic(Server *server, int x, int y, int level, int *players,
                  int nb_players)
{
    char buffer[512];
    char temp[32];

    snprintf(buffer, sizeof(buffer), "pic %d %d %d", x, y, level);
    for (int i = 0; i < nb_players; i++) {
        snprintf(temp, sizeof(temp), " #%d", players[i]);
        strcat(buffer, temp);
    }
    strcat(buffer, "\n");
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_pie(Server *server, int x, int y, int result)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "pie %d %d %d\n", x, y, result);
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_pfk(Server *server, int player_id)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "pfk #%d\n", player_id);
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_pdr(Server *server, int player_id, int resource)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "pdr #%d %d\n", player_id, resource);
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_pgt(Server *server, int player_id, int resource)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "pgt #%d %d\n", player_id, resource);
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_pdi(Server *server, int player_id)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "pdi #%d\n", player_id);
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_enw(Server *server, int egg_id, int player_id, int x, int y)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "enw #%d #%d %d %d\n",
             egg_id, player_id, x, y);
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_ebo(Server *server, int egg_id)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "ebo #%d\n", egg_id);
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_edi(Server *server, int egg_id)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "edi #%d\n", egg_id);
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_seg(Server *server, const char *team_name)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "seg %s\n", team_name);
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_smg(Server *server, const char *message)
{
    char buffer[512];

    snprintf(buffer, sizeof(buffer), "smg %s\n", message);
    // broadcast_to_gui_clients(server, buffer);
}

void send_gui_ppo(Server *server, int player_id)
{
    char buffer[256];
    Player *player;

    if (player_id < 0 || player_id >= server->num_players) {
        return;  // Invalid player ID, don't broadcast
    }

    player = &server->players[player_id];
    snprintf(buffer, sizeof(buffer), "ppo #%d %d %d %d\n",
             player_id, player->x, player->y, player->orientation + 1);
    // broadcast_to_gui_clients(server, buffer);
}

int get_resource_id(const char *resource_name)
{
    static const char *resources[] = {
        "food", "linemate", "deraumere", "sibur",
        "mendiane", "phiras", "thystame", NULL
    };

    for (int i = 0; resources[i] != NULL; i++) {
        if (strcmp(resource_name, resources[i]) == 0) {
            return i;
        }
    }
    return -1;
}

void process_gui_command(Server *server, int client_socket, char *command)
{
    char *cmd = strtok(command, " \n");
    char *arg1, *arg2, *arg3;

    if (!cmd) {
        send(client_socket, "suc\n", 4, 0);
        return;
    }

    if (strcmp(cmd, "msz") == 0) {
        handle_gui_msz(server, client_socket);
    } else if (strcmp(cmd, "bct") == 0) {
        arg1 = strtok(NULL, " ");
        arg2 = strtok(NULL, " ");
        if (arg1 && arg2) {
            handle_gui_bct(server, client_socket, atoi(arg1), atoi(arg2));
        } else {
            send(client_socket, "sbp\n", 4, 0);
        }
    } else if (strcmp(cmd, "mct") == 0) {
        handle_gui_mct(server, client_socket);
    } else if (strcmp(cmd, "tna") == 0) {
        handle_gui_tna(server, client_socket);
    } else if (strcmp(cmd, "ppo") == 0) {
        arg1 = strtok(NULL, " ");
        if (arg1 && arg1[0] == '#') {
            handle_gui_ppo(server, client_socket, atoi(arg1 + 1));
        } else {
            send(client_socket, "sbp\n", 4, 0);
        }
    } else if (strcmp(cmd, "plv") == 0) {
        arg1 = strtok(NULL, " ");
        if (arg1 && arg1[0] == '#') {
            handle_gui_plv(server, client_socket, atoi(arg1 + 1));
        } else {
            send(client_socket, "sbp\n", 4, 0);
        }
    } else if (strcmp(cmd, "pin") == 0) {
        arg1 = strtok(NULL, " ");
        if (arg1 && arg1[0] == '#') {
            handle_gui_pin(server, client_socket, atoi(arg1 + 1));
        } else {
            send(client_socket, "sbp\n", 4, 0);
        }
    } else if (strcmp(cmd, "sgt") == 0) {
        handle_gui_sgt(server, client_socket);
    } else if (strcmp(cmd, "sst") == 0) {
        arg1 = strtok(NULL, " ");
        if (arg1) {
            handle_gui_sst(server, client_socket, atoi(arg1));
        } else {
            send(client_socket, "sbp\n", 4, 0);
        }
    } else {
        send(client_socket, "suc\n", 4, 0);
    }
}