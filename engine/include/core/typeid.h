#pragma once
#include <cstddef>

inline size_t GenerateUniqueTypeID()
{
    static size_t lastID = 0;
    return lastID++;
}

template <typename T> inline size_t GetTypeID()
{
    static const size_t typeID = GenerateUniqueTypeID();
    return typeID;
}
