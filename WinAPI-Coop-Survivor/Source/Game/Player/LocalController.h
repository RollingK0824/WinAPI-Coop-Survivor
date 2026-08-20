#pragma once
#include "Controller.h"
#include "Engine/Core/ObserverPtr.h"

class Player;
class ColliderComponent;
class RigidBodyComponent;

class LocalController : public Controller {
public:
    CLONEABLE_COMPONENT(LocalController)

    LocalController(GameObject* owner, TransformComponent* transform);
    virtual ~LocalController() override = default;

    virtual std::string_view GetComponentType() const override {
        return EngineKey::CustomComponent::LocalController;
    }

    virtual void Start() override;
    virtual void Update(float dt) override;

private:
    void Move(float dt);
    void ApplyMapClamp();

private:
    ObserverPtr<Player> m_pPlayer;
    ObserverPtr<RigidBodyComponent> m_pRigidBody = nullptr;
    ObserverPtr<ColliderComponent> m_pCollider = nullptr;
    float m_SendTimer = 0.0f;
    const float m_SendInterval = 0.033f; // 30Hz

    static constexpr float k_MapHalfWidth  = 2000.0f;
    static constexpr float k_MapHalfHeight = 2000.0f;
};
