/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** client_handling
*/

/*
** EPITECH PROJECT, 2025
** start
** File description:
** client_handling.c
*/

#include "server.h"
#include "time/tick.h"
#include "command/gui_commands.h"
static void add_client_to_fds(server_t *server, int new_socket)
{
    FD_SET(new_socket, &server->master_fds);
    if (new_socket > server->max_fd) {
        server->max_fd = new_socket;
    }
}

static void handle_new_connection(server_t *server)
{
    int new_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[1024];

    new_socket = accept(server->server_socket, (struct sockaddr*)&client_addr,
            &addr_len);
    if (new_socket < 0)
        return;
    printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port));
    snprintf(buffer, 1024, "WELCOME\n");
    send(new_socket, buffer, strlen(buffer), 0);
    add_client_to_fds(server, new_socket);
}

static void handle_client_disconnect(server_t *server, int client_socket)
{
    int player_index = find_player_by_socket(server, client_socket);

    printf("Client disconnected\n");
    close(client_socket);
    FD_CLR(client_socket, &server->master_fds);
    if (player_index != -1) {
        remove_player(server, player_index);
    }
    if (client_socket == server->graphic_fd) {
        printf("Client GRAPHIC déconnecté\n");
        server->graphic_fd = -1;
    }
}

static void clean_message_buffer(char *buffer, int bytes_received)
{
    for (int i = 0; i < bytes_received; i++) {
        if (buffer[i] == '\n' || buffer[i] == '\r') {
            buffer[i] = '\0';
            break;
        }
    }
}

void send_connection_info(server_t *server, int client_socket, int team_id)
{
    char response[1024];

    snprintf(response, 1024, "%d\n%d %d\n",
            server->teams[team_id].max_clients - server->teams[team_id]
            .current_clients, server->width, server->height);
    send(client_socket, response, strlen(response), 0);
}

void send_graphic_init_data(server_t *server, int graphic_fd)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "msz %d %d\n", server->width, server->height);
    send(graphic_fd, buffer, strlen(buffer), 0);
}

static void handle_client_message(server_t *server, int client_socket)
{
    char buffer[1024];
    int bytes_received = recv(client_socket, buffer, 1024 - 1, 0);
    int player_index = find_player_by_socket(server, client_socket);

    if (bytes_received <= 0) {
        handle_client_disconnect(server, client_socket);
        return;
    }
    buffer[bytes_received] = '\0';
    clean_message_buffer(buffer, bytes_received);
    if (client_socket == server->graphic_fd) {
        process_gui_command(server, client_socket, buffer);
        return;
    }
    if (player_index == -1) {
        if (strcmp(buffer, "GRAPHIC") == 0) {
            printf("Client GRAPHIC connecté\n");
            server->graphic_fd = client_socket;
            send_graphic_init_data(server, client_socket);
        } else {
            handle_team_authentication(server, client_socket, buffer);
        }
    } else {
        printf("action add %s\n", buffer);
        add_action_to_queue(&server->players[player_index], buffer, server->freq);
    }
}

void check_new_connections(server_t *server, fd_set *read_fds)
{
    if (FD_ISSET(server->server_socket, read_fds)) {
        handle_new_connection(server);
    }
}

void check_client_messages(server_t *server, fd_set *read_fds)
{
    for (int i = 0; i <= server->max_fd; i++) {
        if (i != server->server_socket && FD_ISSET(i, read_fds)) {
            handle_client_message(server, i);
        }
    }
}

void cleanup_server(server_t *server)
{
    for (int i = 0; i < server->num_players; i++) {
        close(server->players[i].socket);
    }
    close(server->server_socket);
}
