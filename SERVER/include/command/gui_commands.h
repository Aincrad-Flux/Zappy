/*
** EPITECH PROJECT, 2025
** zappy
** File description:
** gui_commands.h
*/

#ifndef GUI_COMMANDS_H_
    #define GUI_COMMANDS_H_

    #include "../server.h"

typedef struct gui_pic_data_s {
    int x;
    int y;
    int level;
    int *players;
    int nb_players;
} gui_pic_data_t;

typedef struct {
    int x;
    int y;
} position_t;

void handle_gui_msz(server_t *server, int client_socket);
void handle_gui_bct(server_t *server, int client_socket, int x, int y);
void handle_gui_mct(server_t *server, int client_socket);
void handle_gui_tna(server_t *server, int client_socket);
void handle_gui_ppo(server_t *server, int client_socket, int player_id);
void handle_gui_plv(server_t *server, int client_socket, int player_id);
void handle_gui_pin(server_t *server, int client_socket, int player_id);
void handle_gui_sgt(server_t *server, int client_socket);

void handle_gui_sst(server_t *server, int client_socket, int new_freq);
void send_gui_pnw(server_t *server, int player_id);
void send_gui_pex(server_t *server, int player_id);
void send_gui_pbc(server_t *server, int player_id, const char *message);
void send_gui_pic(server_t *server, gui_pic_data_t *data);
void send_gui_pie(server_t *server, int x, int y, int result);
void send_gui_pfk(server_t *server, int player_id);
void send_gui_pdr(server_t *server, int player_id, int resource);
void send_to_gui_client(int socket, const char *message);
void send_gui_pgt(server_t *server, int player_id, int resource);
void send_gui_pdi(server_t *server, int player_id);
void send_gui_enw(server_t *server, int egg_id, int player_id, position_t pos);
void send_gui_ebo(server_t *server, int egg_id);
void send_gui_edi(server_t *server, int egg_id);
void send_gui_seg(server_t *server, const char *team_name);
void send_gui_smg(server_t *server, const char *message);
void send_gui_ppo(server_t *server, int player_id);
int get_resource_id(const char *resource_name);

void process_gui_command(server_t *server, int client_socket, char *command);

void broadcast_to_gui_clients(server_t *server, const char *message);

#endif /* !GUI_COMMANDS_H_ */
