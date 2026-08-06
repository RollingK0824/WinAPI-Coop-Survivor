#include "Engine/Core/pch.h"
#include "NetworkController.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/BoxCollider.h"
#include "Game/Player/Player.h"

NetworkController::NetworkController(GameObject* owner, TransformComponent* transform, unsigned int netID)
    : Controller(owner, transform), m_NetID(netID) {}

void NetworkController::Start()
{
    m_pPlayer = gameObject.GetComponent<Player>();
    m_pCollider = gameObject.GetComponent<ColliderComponent>();
}

void NetworkController::Update(float dt) {
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;

    if (NetworkManager::GetInstance()->GetInterpolatedPosition(m_NetID, x, y, angle)) {
        if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId())) {
            b2Vec2 b2Pos = { PixelToMeter(x), PixelToMeter(y) };
            b2Body_SetTransform(m_pCollider->GetBodyId(), b2Pos, b2MakeRot(angle));
            b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f });
        } else {
            transform.SetPosition({ x, y });
            transform.SetRotation(angle);
        }
    }
}
