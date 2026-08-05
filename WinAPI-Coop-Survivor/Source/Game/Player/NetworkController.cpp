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

    // NetworkManager로부터 보간된 좌표 및 각도 조회
    if (NetworkManager::GetInstance()->GetInterpolatedPosition(m_NetID, x, y, angle)) {
        if (m_pCollider && b2Body_IsValid(m_pCollider->GetBodyId())) {
            // Box2D 물리 엔진에 강제 좌표 주입
            b2Vec2 b2Pos = { PixelToMeter(x), PixelToMeter(y) };
            b2Body_SetTransform(m_pCollider->GetBodyId(), b2Pos, b2MakeRot(angle));
            b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f }); // 원격 플레이어이므로 누적 속도 0으로 설정
        } else {
            // 물리 바디가 없으면 직접 Transform에 적용
            transform.SetPosition({ x, y });
            transform.SetRotation(angle);
        }
    }
}
