#include "Engine/Core/pch.h"
#include "InGameManager.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Manager/PrefabManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Network/NetworkIdentity.h"
#include "Game/Player/Player.h"

static ComponentRegistrar<InGameManager> registrar(EngineKey::CustomComponent::InGameManager.data());

InGameManager::InGameManager(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner, transform)
{
}

void InGameManager::Start()
{
	NetworkManager* net = NetworkManager::GetInstance();
	uint32 myNetID = net->GetMyNetID();

	if (myNetID != 0)
	{
		SpawnPlayer(myNetID, true, { 0.0f, 0.0f });
	}

	if (net->GetRole() == NetRole::HOST)
	{
		for (const auto& [clientNetID, clientInfo] : net->GetConnectedClients())
		{
			SpawnPlayer(clientNetID, false, { 100.0f, 0.0f });
		}
	}

	net->RegisterPacketHandler(PacketType::ENTITY_STATE_SYNC,
		[this](const PacketHeader* packet, const sockaddr_in& sender) {
			auto syncPkt = reinterpret_cast<const EntityStateSyncPacket*>(packet);
			uint32 myID = NetworkManager::GetInstance()->GetMyNetID();

			for (int i = 0; i < syncPkt->entityCount; ++i)
			{
				const EntitySyncData& entity = syncPkt->entities[i];
				
				if (!NetworkManager::GetInstance()->GetNetworkObject(entity.netID))
				{
					bool isLocal = (entity.netID == myID);
					this->SpawnPlayer(entity.netID, isLocal, { entity.posX, entity.posY });
				}
			}
		});
}

void InGameManager::Update(float dt)
{
	NetworkManager* net = NetworkManager::GetInstance();
	if (!net->IsConnected()) return;
}

GameObject* InGameManager::SpawnPlayer(uint32 netId, bool isLocal, Vector2 spawnPos)
{
	if (GameObject* existingObj = NetworkManager::GetInstance()->GetNetworkObject(netId))
	{
		return existingObj;
	}

	Scene* pScene = gameObject.GetOwnerScene();
	if (!pScene) return nullptr;

	GameObject* pPlayerObj = PrefabManager::GetInstance()->Instantiate("PlayerPrefab", pScene);
	if (!pPlayerObj)
	{
		return nullptr;
	}

	pPlayerObj->transform.SetPosition(spawnPos);

	NetworkIdentity* netIdentity = pPlayerObj->GetComponent<NetworkIdentity>();
	if (!netIdentity)
	{
		netIdentity = pPlayerObj->AddComponent<NetworkIdentity>();
	}
	netIdentity->SetNetID(netId);
	netIdentity->SetLocalPlayer(isLocal);

	m_playerObjects[netId] = pPlayerObj;

	return pPlayerObj;
}
