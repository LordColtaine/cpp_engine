#include "memory/memorypool.h"
#include "logger/logger.h"
#include <cstdlib>
#include <string>

MemoryPool::~MemoryPool()
{
    const size_t totalFreed = GetTotalMemory();

    for (void* page : m_Pages)
    {
        std::free(page);
    }

    LOG_INFO("Deleted all memory pages in MemoryPool. Freed a total of " + std::to_string(totalFreed) + " bytes.");
}

bool MemoryPool::Init(const size_t chunkSize, const size_t chunksPerPage)
{
    const size_t ChunkStructSize = sizeof(Chunk*);
    m_ChunkSize = chunkSize > ChunkStructSize ? chunkSize : ChunkStructSize;
    m_ChunksPerPage = chunksPerPage;

    const size_t alignment = sizeof(void*);
    const size_t remainder = m_ChunkSize % alignment;
    if (remainder != 0)
    {
        m_ChunkSize += (alignment - remainder);
    }

    return true;
}

void MemoryPool::ExpandPool()
{
    const size_t totalMemory = m_ChunkSize * m_ChunksPerPage;
    void* newPage = std::malloc(totalMemory);
    m_Pages.push_back(newPage);

    LOG_WARN("MemoryPool expanded! Allocated a new page of " + std::to_string(totalMemory) +
             " bytes. Total Pool Size is now: " + std::to_string(GetTotalMemory()) + " bytes.");

    m_Head = reinterpret_cast<Chunk*>(newPage);
    Chunk* cur = m_Head;

    for (size_t i = 0; i < m_ChunksPerPage - 1; ++i)
    {
        char* curAddr = reinterpret_cast<char*>(cur);
        char* nextAddr = curAddr + m_ChunkSize;

        Chunk* next = reinterpret_cast<Chunk*>(nextAddr);
        cur->m_Next = next;
        cur = cur->m_Next;
    }

    cur->m_Next = nullptr;
}

size_t MemoryPool::GetTotalMemory() const
{
    // Total memory = Number of pages * Size of each page
    return m_Pages.size() * (m_ChunkSize * m_ChunksPerPage);
}

void* MemoryPool::Allocate()
{
    if (m_Head == nullptr)
    {
        ExpandPool();
    }

    Chunk* memory = m_Head;
    m_Head = m_Head->m_Next;
    return static_cast<void*>(memory);
}

bool MemoryPool::Free(void* ptr)
{
    if (ptr == nullptr)
    {
        return false;
    }

    Chunk* chunk = reinterpret_cast<Chunk*>(ptr);
    chunk->m_Next = m_Head;
    m_Head = chunk;
    return true;
}
