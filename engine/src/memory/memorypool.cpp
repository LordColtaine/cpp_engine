#include "memory/memorypool.h"
#include <cstdlib>

MemoryPool::~MemoryPool()
{
    // Loop through every page we ever requested and give it back to the OS
    for (void* page : m_Pages)
    {
        std::free(page);
    }
    MEM_LOG("Deleted all memory pages");
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

    MEM_LOG("Pool was empty! Allocated a new page of " << totalMemory << " bytes");

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
