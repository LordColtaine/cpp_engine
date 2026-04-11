#pragma once
#include <iostream>
#include <vector>

class MemoryPool
{
public:
    MemoryPool() = default;
    ~MemoryPool();
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    bool Init(const size_t chunkSize, const size_t chunksPerPage);
    void* Allocate();
    bool Free(void* ptr);

private:
    void ExpandPool();
    size_t GetTotalMemory() const;

    struct Chunk
    {
        Chunk* m_Next = nullptr;
    };

    Chunk* m_Head = nullptr;
    size_t m_ChunkSize = 0;
    size_t m_ChunksPerPage = 0;

    std::vector<void*> m_Pages;
};
