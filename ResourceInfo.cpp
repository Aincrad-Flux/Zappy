#include "ResourceInfo.hpp"

ResourceInfo::ResourceInfo(const char* name, Color color, const char* shape)
    : m_name(name), m_color(color), m_shape(shape)
{
}

ResourceInfo::~ResourceInfo()
{
}
