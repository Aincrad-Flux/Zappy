/*
** EPITECH PROJECT, 2025
** start
** File description:
** command.h
*/

#ifndef COMMAND
    #define COMMAND

    #include <string.h>
    #include <stdio.h>
    #include "../player.h"
    #include <sys/types.h>
    #include <sys/socket.h>

typedef struct Player player_t;
typedef struct Server server_t;

typedef struct {
    const char *name;
    int duration;
} commandduration_t;

int get_command_duration(const char *command);
void move_player_forward(player_t *player, server_t *server);
void handle_inventory_command(player_t *player, char *response);
void handle_set_command(player_t *player, server_t *server,
    const char *command, char *reponse);
void handle_take_command(player_t *player, server_t *server,
    const char *command, char *reponse);
void handle_eject_command(player_t *player, server_t *server, char *reponse);
void handle_broadcast_command(player_t *player, server_t *server, const char *
    command, char *reponse);
void handle_incantation_command(player_t *player, server_t *server,
    char *reponse);
void handle_fork_command(player_t *player, server_t *server, char *reponse);
void process_player_command(player_t *player, server_t *server,
    const char *command);
void handle_look_command(player_t *player, server_t *server, char *response);
void prepare_incantation(player_t *player, server_t *server, char *response);
void finish_incantation(player_t *player, server_t *server);
#endif
