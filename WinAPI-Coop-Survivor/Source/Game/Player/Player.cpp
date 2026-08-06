#include "Engine/Core/pch.h"
#include "Player.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Framework/Components/Network/NetworkIdentity.h"
#include "Engine/Framework/Components/Physics/BoxCollider.h"

static ComponentRegistrar<Player> registrar(EngineKey::CustomComponent::Player.data());

Player::Player(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner, transform) {}

void Player::Start()
{
	m_pCollider = gameObject.GetComponent<ColliderComponent>();

	NetworkIdentity* netIdentity = gameObject.GetComponent<NetworkIdentity>();
	if (!netIdentity) return;

	if (netIdentity->HasAuthority())
	{
		//gameObject.AddComponent<LocalController>();
	}
	else
	{
		//gameObject.AddComponent<NetworkController>(netIdentity->GetNetID());

		if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
		{
			b2Body_SetType(m_pCollider->GetBodyId(), b2_kinematicBody);
		}
	}
}

void Player::Update(float dt)
{
}
