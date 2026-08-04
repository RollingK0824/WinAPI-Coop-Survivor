#pragma once
#include "Engine/Framework/Base/Component.h"

class UIComponent : public Component
{
public:
    UIComponent(GameObject* owner, TransformComponent* transform)
        : Component(owner, transform) {
    }
    virtual ~UIComponent() override = default;

    virtual void Serialize(json& outJson) const override
    {
        Component::Serialize(outJson);
    }
    virtual void Deserialize(const json& inJson) override
    {
        Component::Deserialize(inJson);
    }

    virtual void RenderUI() = 0;
};