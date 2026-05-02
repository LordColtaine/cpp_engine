#pragma once
#include "memory/binnedallocator.h"
#include <utility>
#include <vector>

enum class NodeState
{
    Running,
    Success,
    Failure
};

// ==========================================
// THE BASE NODE
// ==========================================
class BTNode
{
public:
    virtual ~BTNode() = default;
    virtual NodeState Tick(double dt) = 0;
    virtual size_t GetMemorySize() const = 0;
};

// ==========================================
// COMPOSITE NODE
// ==========================================
class CompositeNode : public BTNode
{
public:
    void AddChild(BTNode* child) { m_Children.push_back(child); }

protected:
    std::vector<BTNode*> m_Children;
};

// ==========================================
// THE SELECTOR
// ==========================================
class Selector : public CompositeNode
{
public:
    NodeState Tick(double dt) override
    {
        for (BTNode* child : m_Children)
        {
            const NodeState state = child->Tick(dt);

            if (state == NodeState::Running)
                return NodeState::Running;
            if (state == NodeState::Success)
                return NodeState::Success;
        }

        return NodeState::Failure;
    }

    size_t GetMemorySize() const override { return sizeof(*this); }
};

// ==========================================
// THE SEQUENCE
// ==========================================
class Sequence : public CompositeNode
{
public:
    NodeState Tick(double dt) override
    {
        for (BTNode* child : m_Children)
        {
            const NodeState state = child->Tick(dt);

            if (state == NodeState::Running)
                return NodeState::Running;
            if (state == NodeState::Failure)
                return NodeState::Failure;
        }

        return NodeState::Success;
    }

    size_t GetMemorySize() const override { return sizeof(*this); }
};

// ==========================================
// THE TREE MANAGER (Memory Arena)
// ==========================================
class BehaviorTree
{
public:
    BehaviorTree(BinnedAllocator* allocator) : m_Allocator(allocator), m_Root(nullptr) {}

    ~BehaviorTree();

    void SetRoot(BTNode* root) { m_Root = root; }

    void Tick(double dt)
    {
        if (m_Root != nullptr)
        {
            m_Root->Tick(dt);
        }
    }

    // Custom Factory Method utilizing your memory pool
    template <typename T, typename... Args> T* CreateNode(Args&&... args)
    {
        void* memory = m_Allocator->Allocate(sizeof(T));
        if (memory == nullptr)
            return nullptr;

        T* node = new (memory) T(std::forward<Args>(args)...);
        m_AllNodes.push_back(node);
        return node;
    }

private:
    BinnedAllocator* m_Allocator;
    BTNode* m_Root;
    std::vector<BTNode*> m_AllNodes;
};
