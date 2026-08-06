#include "Engine/Core/pch.h"
#include "NetworkIdentity.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Network/NetworkManager.h"
static ComponentRegistrar<NetworkIdentity>registrar(EngineKey::Component::NetworkIdentity.data());

NetworkIdentity::NetworkIdentity(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner,transform)
{
}

NetworkIdentity::~NetworkIdentity()
{
	if (m_netID != 0)
	{
		NetworkManager::GetInstance()->UnRegisterNetworkObject(m_netID);
	}
}

void NetworkIdentity::Start()
{
	if (m_netID != 0)
	{
		NetworkManager::GetInstance()->RegisterNetworkObject(m_netID, &gameObject);
	}
}
