/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** tick.c
*/

#include "../../../include/server/server.h"
#include "../../../include/server/map/resource.h"

void update_ticks(Server *server)
{
    time_t now = time(NULL);

    if (now - server->last_tick >= 1) {
        server->last_tick = now;
        server->tick_count += 1;
        if (server->tick_count % 20 == 0)
            respawn_resource(server->map);
    }
}
