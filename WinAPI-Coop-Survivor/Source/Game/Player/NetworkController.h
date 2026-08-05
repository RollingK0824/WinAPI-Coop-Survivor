#pragma once
#include "Controller.h"

class Player;

class NetworkController : public Controller {
public:
    CLONEABLE_COMPONENT(NetworkController)

    NetworkController(GameObject* owner, TransformComponent* transform, unsigned int netID);
    virtual ~NetworkController() override = default;

    virtual std::string_view GetComponentType() const override {
        return EngineKey::CustomComponent::NetworkController;
    }
    
    virtual void Start() override;
    virtual void Update(float dt) override;

private:
    Player* m_pPlayer = nullptr;
    ColliderComponent* m_pCollider = nullptr;
    unsigned int m_NetID = 0;
};
