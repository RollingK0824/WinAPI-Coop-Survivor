#include "Engine/Core/pch.h"
#include "Player.h"
#include "LocalController.h"
#include "NetworkController.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/ActionManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"
#include "Engine/Framework/Components/Physics/BoxCollider.h"

static ComponentRegistrar<Player> registrar(EngineKey::CustomComponent::Player.data());

Player::Player(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner, transform) {}

void Player::Start()
{
	m_pCollider = gameObject.GetComponent<ColliderComponent>();

	InitializeNetworkController(m_NetID, m_bIsLocal);
}

void Player::InitializeNetworkController(unsigned int netID, bool isLocal)
{
	m_NetID = netID;
	m_bIsLocal = isLocal;

	if (m_bIsLocal)
	{
		gameObject.AddComponent<LocalController>();
	}
	else
	{
		gameObject.AddComponent<NetworkController>(m_NetID);

		if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
		{
			b2Body_SetType(m_pCollider->GetBodyId(), b2_kinematicBody);
		}
	}
}

void Player::Update(float dt)
{
	// 실제 이동 로직은 각 Controller 컴포넌트가 담당함
}

