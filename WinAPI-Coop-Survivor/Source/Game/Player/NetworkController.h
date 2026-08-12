#pragma once
#include "Controller.h"
#include "Engine/Core/ObserverPtr.h"

class Player;

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

private:
    ObserverPtr<Player> m_pPlayer;
    ObserverPtr<ColliderComponent> m_pCollider;
    uint32 m_NetID = 0;
};
