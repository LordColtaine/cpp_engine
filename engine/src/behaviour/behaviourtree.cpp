#include "behaviour/behaviourtree.h"

BehaviorTree::~BehaviorTree()
{
    for (int i = static_cast<int>(m_AllNodes.size()) - 1; i >= 0; --i)
    {
        BTNode* node = m_AllNodes[i];
        const size_t size = node->GetMemorySize();
        node->~BTNode();
        m_Allocator->Free(node, size);
    }

    m_AllNodes.clear();
}
