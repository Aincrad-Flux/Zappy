/*
** EPITECH PROJECT, 2025
** B-YEP-400-LIL-4-1-zappy-thibault.pouch
** File description:
** resource.h
*/

#ifndef RESOURCE
#define RESOURCE
#include "map.h"

#define RESOURCE_COUNT 7

enum Orientation { NORTH, EAST, SOUTH, WEST };

typedef enum {
    FOOD = 0,
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