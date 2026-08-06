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
	else if (net->GetRole() == NetRole::CLIENT)
	{
		net->RegisterPacketHandler(PacketType::HOST_WELCOME,
			[this](const PacketHeader* packet, const sockaddr_in& sender) {
				auto welcome = reinterpret_cast<const WelcomePacket*>(packet);
				this->SpawnPlayer(welcome->assignedNetID, true, { 0.0f, 0.0f });
			});
	}

	if (net->GetRole() == NetRole::HOST)
	{
		for (const auto& [clientNetID, clientInfo] : net->GetConnectedClients())
		{
			SpawnPlayer(clientNetID, false, { 100.0f, 0.0f });
		}

		net->RegisterPacketHandler(PacketType::PLAYER_INPUT,
			[this](const PacketHeader* packet, const sockaddr_in& sender) {
				auto inputPkt = reinterpret_cast<const PlayerInputPacket*>(packet);
				uint32 clientNetID = inputPkt->netID;

				if (clientNetID != 0 && !NetworkManager::GetInstance()->GetNetworkObject(clientNetID))
				{
					this->SpawnPlayer(clientNetID, false, { inputPkt->posX, inputPkt->posY });
				}

				NetworkManager::GetInstance()->UpdateInterpolationTarget(
					clientNetID, inputPkt->posX, inputPkt->posY, inputPkt->angle);
			});
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

	// CLIENT_DISCONN 수신 시 해당 플레이어 씬 및 매니저 맵에서 제거 (Despawn)
	net->RegisterPacketHandler(PacketType::CLIENT_DISCONN,
		[this](const PacketHeader* packet, const sockaddr_in& sender) {
			auto disconnPkt = reinterpret_cast<const ClientDisconnPacket*>(packet);
			uint32 deadNetID = disconnPkt->disconnectedNetID;

			auto iter = m_playerObjects.find(deadNetID);
			if (iter != m_playerObjects.end())
			{
				Scene* pScene = gameObject.GetOwnerScene();
				if (pScene && iter->second)
				{
					pScene->DestroyObjects(iter->second);
				}
				m_playerObjects.erase(iter);
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
