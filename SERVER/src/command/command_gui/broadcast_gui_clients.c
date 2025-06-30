/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** broadcast_to_gui_clients
*/

#include "server.h"
#include "command/gui_commands.h"

void send_to_gui_client(int socket, const char *message)
{
    ssize_t result = send(socket, message, strlen(message), 0);

    if (result == -1) {
        printf("Erreur envoi vers client GUI socket %d\n", socket);
    }
}

void broadcast_to_gui_clients(server_t *server, const char *message)
{
    if (server->graphic_fd != -1)
        send_to_gui_client(server->graphic_fd, message);
}

static void send_to_ai_client(int socket, const char *message)
{
    ssize_t result;

    result = send(socket, message, strlen(message), 0);
    if (result == -1) {
        printf("Erreur envoi vers client IA socket %d\n", socket);
    }
}

static int is_valid_ai_client(player_t *player)
{
    return (player->team_id != -1 && player->socket > 0);
}

void broadcast_to_ai_clients(server_t *server, const char *message)
{
    int i;

    for (i = 0; i < server->num_players; i++) {
        if (is_valid_ai_client(&server->players[i])) {
            send_to_ai_client(server->players[i].socket, message);
        }
    }
}

static int get_wrapped_distance(int coord1, int coord2, int max_size)
{
    int direct_dist;
    int wrap_dist;

    direct_dist = coord1 - coord2;
    wrap_dist = direct_dist > 0 ? direct_dist - max_size :
        direct_dist + max_size;
    return (abs(direct_dist) <= abs(wrap_dist)) ? direct_dist : wrap_dist;
}

int calculate_sound_direction(server_t *server, player_t *sender,
    player_t *receiver)
{
    int dx;
    int dy;
    double angle;
    int direction;

    if (sender->x == receiver->x && sender->y == receiver->y) {
        return 0;
    }
    dx = get_wrapped_distance(sender->x, receiver->x, server->width);
    dy = get_wrapped_distance(sender->y, receiver->y, server->height);
    angle = atan2(dy, dx);
    direction = ((int)(angle * 4 / (2 * M_PI)) + receiver->orientation) % 8;
    return direction + 1;
}

static void send_message_to_player(server_t *server, int sender_id,
    int receiver_id, const char *text)
{
    char buffer[512];
    int direction;

    direction = calculate_sound_direction(server,
        &server->players[sender_id], &server->players[receiver_id]);
    snprintf(buffer, sizeof(buffer), "message %d, %s\n", direction, text);
    send(server->players[receiver_id].socket, buffer, strlen(buffer), 0);
}

void handle_player_broadcast(server_t *server, int sender_id,
    const char *text)
{
    int i;

    for (i = 0; i < server->num_players; i++) {
        if (i != sender_id && server->players[i].team_id != -1 &&
            server->players[i].socket > 0) {
            send_message_to_player(server, sender_id, i, text);
        }
    }
    send_gui_pbc(server, sender_id, text);
}
