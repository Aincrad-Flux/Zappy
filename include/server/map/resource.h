/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** resource.h
*/

#ifndef RESOURCE
#define RESOURCE
#include "map.h"

typedef enum {
    FOOD,
    LINEMATE,
    DERAUMERE,
    SIBUR,
    MENDIANE,
    PHIRAS,
    THYSTAME
} ResourceType;

void init_ressources(Map *map);
void respawn_resource(Map *map);

#endif