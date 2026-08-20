#pragma once
#include "Controller.h"
#include "Engine/Core/ObserverPtr.h"

class Player;
class ColliderComponent;
class RigidBodyComponent;

class NetworkController : public Controller {
public:
    CLONEABLE_COMPONENT(NetworkController)

    NetworkController(GameObject* owner, TransformComponent* transform, uint32 netID);
    virtual ~NetworkController() override = default;

    virtual std::string_view GetComponentType() const override {
        return EngineKey::CustomComponent::NetworkController;
    }
    
    virtual void Start() override;
    virtual void Update(float dt) override;

    void SetVelocity(const Vector2& vel) { m_velocity = vel; }

private:
    ObserverPtr<Player> m_pPlayer;
    ObserverPtr<RigidBodyComponent> m_pRigidBody;
    ObserverPtr<ColliderComponent> m_pCollider;

    Vector2 m_velocity{ 0.0f,0.0f };
    uint32 m_NetID = 0;
};
