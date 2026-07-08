#ifndef MOVE_LOG_H
#define MOVE_LOG_H
#include "move_entry.h"

struct MoveLog
{
    std::array<MoveEntry, MOVE_ENTRIES_CAPACITY> m_entries;
    unsigned int m_writeIndex = 0;
    unsigned int m_count = 0;

    MoveEntry &operator[](size_t i)
    {
        return m_entries[i];
    }

    const MoveEntry &operator[](size_t i) const
    {
        return m_entries[i];
    }
};
#endif