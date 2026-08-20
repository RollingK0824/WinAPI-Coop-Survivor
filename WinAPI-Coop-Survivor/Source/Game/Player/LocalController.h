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

    // InGameScene: 4 tiles of 1024x1024 at (+-510, +-510) -> Map extent: [-1022, 1022]
    // Player size: 34x34 (half-extent: 17.0f) -> Clamped bounds: [-1005, 1005]
    static constexpr float k_MapHalfWidth  = 1005.0f;
    static constexpr float k_MapHalfHeight = 1005.0f;
};
