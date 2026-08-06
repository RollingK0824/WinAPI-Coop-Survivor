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
#include "Game/Player/Player.h"

LocalController::LocalController(GameObject* owner, TransformComponent* transform)
    : Controller(owner, transform) {}

void LocalController::Start() {
    m_pPlayer = gameObject.GetComponent<Player>();
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

    // 내 위치와 속도를 60Hz 주기로 Host에게 전송
    m_SendTimer += dt;
    if (m_SendTimer >= m_SendInterval) {
        m_SendTimer = 0.0f;

        NetworkManager* net = NetworkManager::GetInstance();
        if (net->GetRole() != NetRole::NONE && net->IsConnected()) {
            if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId())) {
                b2Vec2 pos = b2Body_GetPosition(m_pCollider->GetBodyId());
                b2Vec2 vel = b2Body_GetLinearVelocity(m_pCollider->GetBodyId());
                float angle = b2Rot_GetAngle(b2Body_GetRotation(m_pCollider->GetBodyId()));

                PlayerInputPacket packet;
                packet.header.type = PacketType::PLAYER_INPUT;
                packet.header.size = sizeof(PlayerInputPacket);
                packet.netID = net->GetMyNetID();
                packet.posX = MeterToPixel(pos.x);
                packet.posY = MeterToPixel(pos.y);
                packet.velX = MeterToPixel(vel.x);
                packet.velY = MeterToPixel(vel.y);
                packet.angle = angle;

                net->SendPacket(&packet, sizeof(PlayerInputPacket));
            }
        }
    }
}

void LocalController::Move(float dt) {
    if (!m_pCollider.IsValid() || !b2Body_IsValid(m_pCollider->GetBodyId())) {
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

    b2Vec2 b2Velocity = { PixelToMeter(targetVelocity.x), PixelToMeter(targetVelocity.y) };
    b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), b2Velocity);
}
