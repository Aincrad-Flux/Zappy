/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** Resource.hpp
*/

#pragma once

#include <raylib.h>
#include <string>

/**
 * @enum ResourceType
 * @brief Types of resources available in the game
 */
enum class ResourceType {
    FOOD = 0,      ///< Food resource
    LINEMATE = 1,  ///< Linemate resource
    DERAUMERE = 2, ///< Deraumere resource
    SIBUR = 3,     ///< Sibur resource
    MENDIANE = 4,  ///< Mendiane resource
    PHIRAS = 5,    ///< Phiras resource
    THYSTAME = 6   ///< Thystame resource
};

/**
 * @class Resource
 * @brief Represents a resource item in the game world
 *
 * This class handles resource rendering and properties.
 */
class Resource {
private:
    ResourceType type;    ///< Type of the resource
    Vector3 position;     ///< Position in 3D space
    Color color;          ///< Color representation
    std::string name;     ///< Resource name
    int count;            ///< Number of resources of this type

public:
    /**
     * @brief Constructor for Resource
     * @param resType Type of the resource
     * @param pos Position in the world
     */
    Resource(ResourceType resType, Vector3 pos);

    /**
     * @brief Destructor for Resource
     */
    ~Resource();

    /**
     * @brief Draws the resource in the world
     * @param worldPos Base position in the world
     * @param tileSize Size of a map tile
     */
    void draw(Vector3 worldPos, int tileSize) const;

    /**
     * @brief Gets the resource type
     * @return The resource type
     */
    ResourceType getType() const;

    /**
     * @brief Gets the resource position
     * @return The position vector
     */
    Vector3 getPosition() const;

    /**
     * @brief Gets the resource color
     * @return The color
     */
    Color getColor() const;
    std::string getName() const;

    /**
     * @brief Gets the count of resources
     * @return The number of resources of this type
     */
    int getCount() const;

    /**
     * @brief Sets the count of resources
     * @param newCount The new number of resources
     */
    void setCount(int newCount);

    static Color getResourceColor(ResourceType type);
    static std::string getResourceName(ResourceType type);
};