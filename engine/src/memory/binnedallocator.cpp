#include "memory/binnedallocator.h"

#include <iostream>

void BinnedAllocator::Init()
{
    m_Bin32.Init(32, 10);
    m_Bin64.Init(64, 150000);
    m_Bin128.Init(128, 10);
    m_Bin256.Init(256, 100);
}

void* BinnedAllocator::Allocate(const size_t size)
{
    if (size <= 32)
    {
        MEM_LOG("Allocating in Bin32");
        return m_Bin32.Allocate();
    }
    else if (size <= 64)
    {
        MEM_LOG("Allocating in Bin64");
        return m_Bin64.Allocate();
    }
    else if (size <= 128)
    {
        MEM_LOG("Allocating in Bin128");
        return m_Bin128.Allocate();
    }
    else if (size <= 256)
    {
        MEM_LOG("Allocating in Bin256");
        return m_Bin256.Allocate();
    }
    else
    {
        std::cout << "[MEM] Fallback to malloc for massive object!" << std::endl;
        return malloc(size);
    }
}

void BinnedAllocator::Free(void* ptr, const size_t size)
{
    if (size <= 32)
    {
        m_Bin32.Free(ptr);
    }
    else if (size <= 64)
    {
        m_Bin64.Free(ptr);
    }
    else if (size <= 128)
    {
        m_Bin128.Free(ptr);
    }
    else if (size <= 256)
    {
        m_Bin256.Free(ptr);
    }
    else
    {
        free(ptr);
    }
}
