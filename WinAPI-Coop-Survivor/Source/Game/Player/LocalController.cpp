#include "Engine/Core/pch.h"
#include "LocalController.h"
#include "Engine/Manager/CameraManager.h"
#include "Engine/Manager/ActionManager.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Renderer/RenderGizmo.h" 
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/CameraComponent.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/BoxCollider.h"
#include "Engine/Framework/Components/Physics/RigidBodyComponent.h"
#include "Game/Player/Player.h"

LocalController::LocalController(GameObject* owner, TransformComponent* transform)
    : Controller(owner, transform) {}

void LocalController::Start() {
    m_pPlayer = gameObject.GetComponent<Player>();
    m_pRigidBody = gameObject.GetComponent<RigidBodyComponent>();
    m_pCollider = gameObject.GetComponent<ColliderComponent>();

    ActionManager::GetInstance()->BindAction("MoveUp", VK_UP);
    ActionManager::GetInstance()->BindAction("MoveUp", 'W');
    ActionManager::GetInstance()->BindAction("MoveDown", VK_DOWN);
    ActionManager::GetInstance()->BindAction("MoveDown", 'S');
    ActionManager::GetInstance()->BindAction("MoveLeft", VK_LEFT);
    ActionManager::GetInstance()->BindAction("MoveLeft", 'A');
    ActionManager::GetInstance()->BindAction("MoveRight", VK_RIGHT);
    ActionManager::GetInstance()->BindAction("MoveRight", 'D');

    CameraComponent* pMainCamera = CameraManager::GetInstance()->GetMainCamera();
    if (pMainCamera)
    {
        pMainCamera->SetTarget(&gameObject);
    }
}

void LocalController::Update(float dt) {
    Move(dt);

    m_SendTimer += dt;
    if (m_SendTimer >= m_SendInterval) {
        m_SendTimer = 0.0f;

        NetworkManager* net = NetworkManager::GetInstance();
        if (net->GetRole() != NetRole::NONE && net->IsConnected()) {
            b2BodyId bodyId = b2_nullBodyId;
            if (m_pRigidBody.IsValid() && b2Body_IsValid(m_pRigidBody->GetBodyId()))
                bodyId = m_pRigidBody->GetBodyId();
            else if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
                bodyId = m_pCollider->GetBodyId();

            if (b2Body_IsValid(bodyId)) {
                b2Vec2 pos = b2Body_GetPosition(bodyId);
                b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
                float angle = b2Rot_GetAngle(b2Body_GetRotation(bodyId));

                PlayerInputPacket packet;
                packet.header.type = PacketType::PLAYER_INPUT;
                packet.header.size = sizeof(PlayerInputPacket);
                packet.netID = net->GetMyNetID();
                packet.pos = Vector2(MeterToPixel(pos.x), MeterToPixel(pos.y));
                packet.vel = Vector2(MeterToPixel(vel.x), MeterToPixel(vel.y));
                packet.angle = angle;

                net->SendPacket(&packet, sizeof(PlayerInputPacket));
            }
        }
    }
}

void LocalController::Move(float dt) {
    b2BodyId bodyId = b2_nullBodyId;
    if (m_pRigidBody.IsValid() && b2Body_IsValid(m_pRigidBody->GetBodyId()))
        bodyId = m_pRigidBody->GetBodyId();
    else if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
        bodyId = m_pCollider->GetBodyId();

    if (!b2Body_IsValid(bodyId)) {
        return;
    }

    Vector2 targetVelocity = { 0.0f, 0.0f };

    if (ActionManager::GetInstance()->GetActionPress("MoveUp"))
        targetVelocity.y -= m_pPlayer->GetSpeed();
    if (ActionManager::GetInstance()->GetActionPress("MoveDown"))
        targetVelocity.y += m_pPlayer->GetSpeed();
    if (ActionManager::GetInstance()->GetActionPress("MoveLeft"))
        targetVelocity.x -= m_pPlayer->GetSpeed();
    if (ActionManager::GetInstance()->GetActionPress("MoveRight"))
        targetVelocity.x += m_pPlayer->GetSpeed();

    bool isMoving = (targetVelocity.LengthSquared() > 0.0001f);
    if (m_pPlayer.IsValid())
    {
        m_pPlayer->SetMoving(isMoving);
        if (isMoving)
        {
            m_pPlayer->SetFacingDirection(targetVelocity);
        }
    }

    b2Vec2 b2Velocity = { PixelToMeter(targetVelocity.x), PixelToMeter(targetVelocity.y) };
    b2Body_SetLinearVelocity(bodyId, b2Velocity);

    ApplyMapClamp();
}

void LocalController::ApplyMapClamp() {
    b2BodyId bodyId = b2_nullBodyId;
    if (m_pRigidBody.IsValid() && b2Body_IsValid(m_pRigidBody->GetBodyId()))
        bodyId = m_pRigidBody->GetBodyId();
    else if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
        bodyId = m_pCollider->GetBodyId();

    if (!b2Body_IsValid(bodyId)) return;

    b2Vec2 pos = b2Body_GetPosition(bodyId);
    float px = MeterToPixel(pos.x);
    float py = MeterToPixel(pos.y);

    float clampedX = std::clamp(px, -k_MapHalfWidth,  k_MapHalfWidth);
    float clampedY = std::clamp(py, -k_MapHalfHeight, k_MapHalfHeight);

    if (clampedX != px || clampedY != py)
    {
        b2Body_SetTransform(
            bodyId,
            { PixelToMeter(clampedX), PixelToMeter(clampedY) },
            b2Body_GetRotation(bodyId)
        );
        b2Body_SetLinearVelocity(bodyId, { 0.0f, 0.0f });
        transform.SetPosition(clampedX, clampedY);
    }
}
