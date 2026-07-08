#ifndef MOVE_ENTRY_H
#define MOVE_ENTRY_H
#include <Arduino.h>
#include <string.h>
#include <array>

// struct to contain information about every move that occured
struct MoveEntry
{
    std::array<char, 20> m_timestamp;

    // allows to directly assign "timestamp" to object
    MoveEntry &operator=(const char *timestamp)
    {
        snprintf(m_timestamp.data(), m_timestamp.size(), "%s|", timestamp);
        return *this;
    }
};
#endif