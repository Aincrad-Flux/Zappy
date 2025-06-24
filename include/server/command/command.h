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

typedef struct Player Player;
typedef struct Server Server;

typedef struct {
    const char *name;
    int duration;
} CommandDuration;

int get_command_duration(const char *command);
void move_player_forward(Player *player, Server *server);
void handle_inventory_command(Player *player, char *response);
void handle_set_command(Player *player, Server *server, const char* command,
    char *reponse);
void handle_take_command(Player *player, Server *server, const char* command,
    char *reponse);
void handle_eject_command(Player *player, Server *server, char *reponse);
void handle_broadcast_command(Player *player, Server *server, const char *
    command, char *reponse);
void handle_incantation_command(Player *player, Server *server,
    char *reponse);
void handle_fork_command(Player *player, Server *server, char *reponse);
void process_player_command(Player *player, Server *server,
                            const char *command);
void handle_look_command(Player *player, Server *server, char *reponse);
#endif