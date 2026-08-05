#pragma once
#include "Controller.h"

class Player;
class ColliderComponent;

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

private:
    Player* m_pPlayer = nullptr;
    ColliderComponent* m_pCollider = nullptr;
    float m_SendTimer = 0.0f;
    const float m_SendInterval = 0.033f; // 30Hz
};
