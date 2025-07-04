/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** Tile.cpp
*/

#include "Tile.hpp"

Tile::Tile(Vector3 position, Color baseColor, bool hasResource,
           bool hasPlayer, int resourceType, int playerCount)
    : m_position(position), m_baseColor(baseColor), m_hasResource(hasResource),
      m_hasPlayer(hasPlayer), m_resourceType(resourceType), m_resourceCount(1), m_playerCount(playerCount)
{
}

Tile::~Tile()
{
}
