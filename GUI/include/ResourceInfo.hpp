/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** ResourceInfo.hpp
*/

#pragma once
#include <raylib.h>

/**
 * @class ResourceInfo
 * @brief Contains information about a resource for display in the UI
 */
class ResourceInfo {
private:
    const char* m_name;   ///< Name of the resource
    Color m_color;        ///< Color representing the resource
    const char* m_shape;  ///< Shape representing the resource

public:
    /**
     * @brief Constructor for ResourceInfo
     * @param name Name of the resource
     * @param color Color representing the resource
     * @param shape Shape representing the resource
     */
    ResourceInfo(const char* name, Color color, const char* shape);

    /**
     * @brief Destructor for ResourceInfo
     */
    ~ResourceInfo();

    // Getters
    const char* getName() const { return m_name; }
    Color getColor() const { return m_color; }
    const char* getShape() const { return m_shape; }
};
