/*
** EPITECH PROJECT, 2025
** zappy
** File description:
** gui_commands.h
*/

#ifndef GUI_COMMANDS_H_
#define GUI_COMMANDS_H_

#include "../server.h"

// GUI Request handlers (Client -> Server)
void handle_gui_msz(Server *server, int client_socket);
void handle_gui_bct(Server *server, int client_socket, int x, int y);
void handle_gui_mct(Server *server, int client_socket);
void handle_gui_tna(Server *server, int client_socket);
void handle_gui_ppo(Server *server, int client_socket, int player_id);
void handle_gui_plv(Server *server, int client_socket, int player_id);
void handle_gui_pin(Server *server, int client_socket, int player_id);
void handle_gui_sgt(Server *server, int client_socket);
void handle_gui_sst(Server *server, int client_socket, int new_freq);

// GUI Notifications (Server -> All GUI Clients)
void send_gui_pnw(Server *server, int player_id);
void send_gui_pex(Server *server, int player_id);
void send_gui_pbc(Server *server, int player_id, const char *message);
void send_gui_pic(Server *server, int x, int y, int level, int *players,
    int nb_players);
void send_gui_pie(Server *server, int x, int y, int result);
void send_gui_pfk(Server *server, int player_id);
void send_gui_pdr(Server *server, int player_id, int resource);
void send_gui_pgt(Server *server, int player_id, int resource);
void send_gui_pdi(Server *server, int player_id);
void send_gui_enw(Server *server, int egg_id, int player_id, int x, int y);
void send_gui_ebo(Server *server, int egg_id);
void send_gui_edi(Server *server, int egg_id);
void send_gui_seg(Server *server, const char *team_name);
void send_gui_smg(Server *server, const char *message);
void send_gui_ppo(Server *server, int player_id);

// Utility functions
int get_resource_id(const char *resource_name);
void process_gui_command(Server *server, int client_socket, char *command);

#endif /* !GUI_COMMANDS_H_ */