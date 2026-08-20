#include "Engine/Core/pch.h"
#include "NetworkController.h"
#include "Engine/Framework/Components/Network/NetworkIdentity.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/BoxCollider.h"
#include "Engine/Framework/Components/Physics/RigidBodyComponent.h"
#include "Game/Player/Player.h"

NetworkController::NetworkController(GameObject* owner, TransformComponent* transform, uint32 netID)
    : Controller(owner, transform), m_NetID(netID) {}

void NetworkController::Start()
{
    m_pPlayer = gameObject.GetComponent<Player>();
    m_pRigidBody = gameObject.GetComponent<RigidBodyComponent>();
    m_pCollider = gameObject.GetComponent<ColliderComponent>();
}

void NetworkController::Update(float dt) {
    NetRole role = NetworkManager::GetInstance()->GetRole();
    Vector2 interpolatedPos;
    NetworkIdentity* netId = gameObject.GetComponent<NetworkIdentity>();

    b2BodyId bodyId = b2_nullBodyId;
    if (m_pRigidBody.IsValid() && b2Body_IsValid(m_pRigidBody->GetBodyId()))
        bodyId = m_pRigidBody->GetBodyId();
    else if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
        bodyId = m_pCollider->GetBodyId();

    if (role == NetRole::HOST) {
        if (b2Body_IsValid(bodyId)) {
            b2Vec2 currentB2Pos = b2Body_GetPosition(bodyId);
            Vector2 currentPos = { MeterToPixel(currentB2Pos.x), MeterToPixel(currentB2Pos.y) };

            Vector2 finalVel = m_velocity;
            bool isMoving = (m_velocity.LengthSquared() > 0.0001f);
            if (m_pPlayer.IsValid())
            {
                m_pPlayer->SetMoving(isMoving);
                if (isMoving)
                {
                    m_pPlayer->SetFacingDirection(m_velocity);
                }
            }

            if (netId && netId->GetInterpolatedPosition(interpolatedPos)) {
                Vector2 posError = interpolatedPos - currentPos;
                float errorDist = posError.Length();

                if (errorDist > 300.0f) {
                    b2Vec2 b2SnapPos = { PixelToMeter(interpolatedPos.x), PixelToMeter(interpolatedPos.y) };
                    b2Body_SetTransform(bodyId, b2SnapPos, b2Rot_identity);
                    currentPos = interpolatedPos;
                } else {
                    constexpr float kCorrectionFactor = 8.0f;
                    finalVel = finalVel + (posError * kCorrectionFactor);
                }
            }

            b2Vec2 b2Vel = { PixelToMeter(finalVel.x), PixelToMeter(finalVel.y) };
            b2Body_SetLinearVelocity(bodyId, b2Vel);

            transform.SetPosition(currentPos);
        }
    } else {
        if (netId && netId->GetInterpolatedPosition(interpolatedPos)) {
            Vector2 currentPos = transform.GetPosition();
            Vector2 delta = interpolatedPos - currentPos;
            bool isMoving = (delta.LengthSquared() > 0.001f);
            if (m_pPlayer.IsValid())
            {
                m_pPlayer->SetMoving(isMoving);
                if (isMoving)
                {
                    m_pPlayer->SetFacingDirection(delta);
                }
            }

            transform.SetPosition(interpolatedPos);
            if (b2Body_IsValid(bodyId)) {
                b2Vec2 b2Pos = { PixelToMeter(interpolatedPos.x), PixelToMeter(interpolatedPos.y) };
                b2Body_SetTransform(bodyId, b2Pos, b2Rot_identity);
            }
        }
    }
}
