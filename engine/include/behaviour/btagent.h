#pragma once
#include "behaviour/behaviourtree.h"
#include "core/gameobject.h"
#include "core/world.h"

class BTAgent : public GameObject
{
public:
    void Initialize() override
    {
        m_Brain = std::make_unique<BehaviorTree>(GetWorld()->GetAllocator());
        ConstructBrain();
    }

    void Update(double dt) override
    {
        if (m_Brain)
        {
            m_Brain->Tick(dt);
        }
    }

protected:
    std::unique_ptr<BehaviorTree> m_Brain;
    virtual void ConstructBrain() = 0;
};
