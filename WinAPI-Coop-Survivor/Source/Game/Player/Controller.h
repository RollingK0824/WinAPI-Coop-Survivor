#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class Controller : public ScriptComponent {
public:
    Controller(GameObject* owner, TransformComponent* transform) 
        : ScriptComponent(owner, transform) {}
    virtual ~Controller() override = default;

    virtual std::string_view GetComponentType() const override {
        return EngineKey::CustomComponent::Controller;
    }

    virtual void Start() override {}
    virtual void Update(float dt) override = 0;
};
