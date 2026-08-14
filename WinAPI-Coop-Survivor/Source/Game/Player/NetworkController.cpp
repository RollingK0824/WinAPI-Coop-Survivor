#include "Engine/Core/pch.h"
#include "NetworkController.h"
#include "Engine/Framework/Components/Network/NetworkIdentity.h"
#include "Engine/Network/NetworkManager.h"
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
    NetRole role = NetworkManager::GetInstance()->GetRole();
    Vector2 interpolatedPos;
    NetworkIdentity* netId = gameObject.GetComponent<NetworkIdentity>();

    if (role == NetRole::HOST) {
        // Host (서버 권한): Soft Correction Velocity (부드러운 보정 속도) 적용으로 Rubber-banding 0% & 몬스터 겹침 0%
        if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId())) {
            b2BodyId bodyId = m_pCollider->GetBodyId();
            b2Vec2 currentB2Pos = b2Body_GetPosition(bodyId);
            Vector2 currentPos = { MeterToPixel(currentB2Pos.x), MeterToPixel(currentB2Pos.y) };

            Vector2 finalVel = m_velocity;

            if (netId && netId->GetInterpolatedPosition(interpolatedPos)) {
                Vector2 posError = interpolatedPos - currentPos;
                float errorDist = posError.Length();

                if (errorDist > 300.0f) {
                    // 300px 이상의 극단적인 순간이동(맵 이동/스폰) 시에만 예외적 스냅
                    b2Vec2 b2SnapPos = { PixelToMeter(interpolatedPos.x), PixelToMeter(interpolatedPos.y) };
                    b2Body_SetTransform(bodyId, b2SnapPos, b2Rot_identity);
                    currentPos = interpolatedPos;
                } else {
                    // 평소에는 오차 벡터에 비례하는 보정 속도를 합산하여 부드럽게 수렴 (Soft Error Decay)
                    constexpr float kCorrectionFactor = 8.0f;
                    finalVel = finalVel + (posError * kCorrectionFactor);
                }
            }

            // 몬스터를 부드럽게 미는 물리 속도 적용 (텔레포트 0회로 고무줄 현상 완전 소멸)
            b2Vec2 b2Vel = { PixelToMeter(finalVel.x), PixelToMeter(finalVel.y) };
            b2Body_SetLinearVelocity(bodyId, b2Vel);

            // 물리 시뮬레이션 연산 좌표로 Transform 갱신
            transform.SetPosition(currentPos);
        }
    } else {
        // Client (원격 플레이어 관람): 보간 위치 적용
        if (netId && netId->GetInterpolatedPosition(interpolatedPos)) {
            transform.SetPosition(interpolatedPos);
            if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId())) {
                b2Vec2 b2Pos = { PixelToMeter(interpolatedPos.x), PixelToMeter(interpolatedPos.y) };
                b2Body_SetTransform(m_pCollider->GetBodyId(), b2Pos, b2Rot_identity);
            }
        }
    }
}
