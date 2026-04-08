#pragma once
#include "memorypool.h"

class BinnedAllocator
{
public:
    void Init();

    void* Allocate(const size_t size);
    void Free(void* ptr, const size_t size);

private:
    MemoryPool m_Bin32;
    MemoryPool m_Bin64;
    MemoryPool m_Bin128;
    MemoryPool m_Bin256;
};
