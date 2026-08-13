#include "Engine/Core/pch.h"
#include "NetworkController.h"
#include "Engine/Framework/Components/Network/NetworkIdentity.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/BoxCollider.h"
#include "Game/Player/Player.h"

NetworkController::NetworkController(GameObject* owner, TransformComponent* transform, uint32 netID)
    : Controller(owner, transform), m_NetID(netID) {}

void NetworkController::Start()
{
    m_pPlayer = gameObject.GetComponent<Player>();
    m_pCollider = gameObject.GetComponent<ColliderComponent>();
}

void NetworkController::Update(float dt) {
    Vector2 interpolatedPos;
    NetworkIdentity* netId = gameObject.GetComponent<NetworkIdentity>();
    if (netId && netId->GetInterpolatedPosition(interpolatedPos)) {
        transform.SetPosition(interpolatedPos);
        if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId())) {
            b2Vec2 b2Pos = { PixelToMeter(interpolatedPos.x), PixelToMeter(interpolatedPos.y) };
            b2Body_SetTransform(m_pCollider->GetBodyId(), b2Pos, b2Rot_identity);
            b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f });
        }
    }
}
