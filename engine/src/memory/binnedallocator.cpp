#include "memory/binnedallocator.h"
#include "logger/logger.h"

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
    std::string sizeStr = std::to_string(size);
    if (size <= 32)
    {
        LOG_INFO("Allocating " + sizeStr + " in 32 byte bin.");
        return m_Bin32.Allocate();
    }
    else if (size <= 64)
    {
        LOG_INFO("Allocating " + sizeStr + " in 64 byte bin.");
        return m_Bin64.Allocate();
    }
    else if (size <= 128)
    {
        LOG_INFO("Allocating " + sizeStr + " in 128 byte bin.");
        return m_Bin128.Allocate();
    }
    else if (size <= 256)
    {
        LOG_INFO("Allocating " + sizeStr + " in 256 byte bin.");
        return m_Bin256.Allocate();
    }
    else
    {
        LOG_WARN("Asking for " + sizeStr + " bytes, so falling back to malloc.");
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
