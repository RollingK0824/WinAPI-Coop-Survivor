#include "Engine/Core/pch.h"
#include "InGameManager.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/TimeManager.h"
#include "Engine/Manager/RandomManager.h"
#include "Engine/Manager/PrefabManager.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Network/NetworkIdentity.h"
#include "Engine/Framework/Components/Physics/ColliderComponent.h"
#include "Engine/Manager/DebugManager.h"
#include "Game/Player/Player.h"
#include "Game/Monster/MonsterSpawner.h"
#include "Game/Skill/SkillComponent.h"

InGameManager* InGameManager::s_instance = nullptr;

static ComponentRegistrar<InGameManager> registrar(EngineKey::CustomComponent::InGameManager.data());

InGameManager::InGameManager(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner, transform)
{
	s_instance = this;
}

InGameManager::~InGameManager()
{
	if (s_instance == this) s_instance = nullptr;
}

void InGameManager::OnDestroy()
{
	ScriptComponent::OnDestroy();
	if (s_instance == this) s_instance = nullptr;
}

void InGameManager::Start()
{
	TimeManager::GetInstance()->SetPaused(false);
	m_countdownTimer = 3.0f;
	m_bIsCountDown = false;
	m_bIsGameStarted = (NetworkManager::GetInstance()->GetRole() == NetRole::NONE);

	Scene* pScene = gameObject.GetOwnerScene();
	if (pScene)
	{
		DebugManager::GetInstance()->CreateDebugUIOverlay(pScene);

		auto initSkillPool = [pScene](const std::string& key, size_t cap) {
			PoolManager::GetInstance()->CreatePool<GameObject>(
				key,
				[key, pScene]() { return PrefabManager::GetInstance()->Instantiate(key, pScene); },
				[](GameObject* obj) { if (obj) obj->SetActive(true); },
				[](GameObject* obj) { if (obj) obj->SetActive(false); },
				nullptr,
				cap, 300
			);
		};

		initSkillPool("GenericProjectilePrefab", 300);
		initSkillPool("GenericAuraPrefab", 20);
		initSkillPool("GenericAoEPrefab", 50);
	}

	if (!gameObject.GetComponent<MonsterSpawner>())
	{
		gameObject.AddComponent<MonsterSpawner>();
	}

	NetworkManager* net = NetworkManager::GetInstance();
	uint32 myNetID = net->GetMyNetID();

	if (myNetID != 0)
	{
		float startX = (static_cast<float>(myNetID) - 1.0f) * 120.0f;
		SpawnPlayer(myNetID, true, { startX, 0.0f });
	}
	else if (net->GetRole() == NetRole::CLIENT)
	{
		net->RegisterPacketHandler(PacketType::HOST_WELCOME,
			[this](const PacketHeader* packet, const sockaddr_in& sender) {
				auto welcome = reinterpret_cast<const WelcomePacket*>(packet);
				float startX = (static_cast<float>(welcome->assignedNetID) - 1.0f) * 120.0f;
				this->SpawnPlayer(welcome->assignedNetID, true, { startX, 0.0f });
			});
	}

	if (net->GetRole() == NetRole::HOST)
	{
		for (const auto& [clientNetID, clientInfo] : net->GetConnectedClients())
		{
			float startX = (static_cast<float>(clientNetID) - 1.0f) * 120.0f;
			SpawnPlayer(clientNetID, false, { startX, 0.0f });
		}

		net->RegisterPacketHandler(PacketType::CLIENT_READY_REQ,
			[this](const PacketHeader* packet, const sockaddr_in& sender) {
				auto readyPkt = reinterpret_cast<const ClientReadyReqPacket*>(packet);
				this->m_clientReadyMap[readyPkt->netId] = readyPkt->isReady;

				if (this->m_onReadyStatusChanged)
				{
					this->m_onReadyStatusChanged(this->IsAllClientsReady());
				}
			});

		net->RegisterPacketHandler(PacketType::PLAYER_INPUT,
			[this](const PacketHeader* packet, const sockaddr_in& sender) {
				auto inputPkt = reinterpret_cast<const PlayerInputPacket*>(packet);
				uint32 clientNetID = inputPkt->netID;

				if (clientNetID != 0 && !NetworkManager::GetInstance()->GetNetworkObject(clientNetID))
				{
					this->SpawnPlayer(clientNetID, false, inputPkt->pos);
				}

				NetworkManager::GetInstance()->UpdateInterpolationTarget(
					clientNetID, inputPkt->pos.x, inputPkt->pos.y, inputPkt->angle);
			});
	}

	net->RegisterPacketHandler(PacketType::GAME_START_SIGNAL,
		[this](const PacketHeader* packet, const sockaddr_in& sender) {
			auto startPkt = reinterpret_cast<const GameStartSignalPacket*>(packet);

			RandomManager::GetInstance()->SetSharedSeed(startPkt->randomSeed);

			float pingMs = NetworkManager::GetInstance()->GetPing();
			float latencySec = (pingMs * 0.5f) / 1000.0f;
			float finalCountdown = startPkt->countdown;
			if (finalCountdown > latencySec)
			{
				finalCountdown -= latencySec;
			}

			this->m_countdownTimer = finalCountdown;
			this->m_bIsCountDown = true;
			this->SortedPlayerCache();
		});

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
					this->SpawnPlayer(entity.netID, isLocal, entity.pos);
				}
			}
		});

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
	if (m_bIsCountDown)
	{
		float unscaledDt = TimeManager::GetInstance()->GetUnscaledDeltaTime();
		m_countdownTimer -= unscaledDt;

		if (m_onCountdownTick)
		{
			m_onCountdownTick(m_countdownTimer);
		}

		if (m_countdownTimer <= 0.0f)
		{
			m_countdownTimer = 0.0f;
			m_bIsCountDown = false;
			m_bIsGameStarted = true;

			this->SortedPlayerCache();

			MonsterSpawner* spawner = gameObject.GetComponent<MonsterSpawner>();
			if (spawner)
			{
				spawner->StartSpawning();
			}

			TimeManager::GetInstance()->SetPaused(false);

			if (m_onGameStarted)
			{
				m_onGameStarted();
			}
		}
	}

	NetworkManager* net = NetworkManager::GetInstance();
	if (!net->IsConnected()) return;
}

void InGameManager::SortedPlayerCache()
{
	m_vCachedPlayer.clear();

	std::vector<std::pair<uint32, GameObject*>> tempPairs;
	for (const auto& [netId, pPlayerObj] : m_playerObjects)
	{
		if (pPlayerObj && pPlayerObj->IsActive())
		{
			tempPairs.push_back({ netId,pPlayerObj });
		}
	}

	std::sort(tempPairs.begin(), tempPairs.end(), [](const auto& a, const auto& b)
		{
			return a.first < b.first;
		});

	for (const auto& pair : tempPairs)
	{
		m_vCachedPlayer.push_back(pair.second);
	}
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

	// 스폰 좌표가 0,0 기본값이면 NetID에 따라 가로 120px 간격으로 일렬 스폰
	if (spawnPos.x == 0.0f && spawnPos.y == 0.0f)
	{
		spawnPos.x = (static_cast<float>(netId) - 1.0f) * 120.0f;
	}

	pPlayerObj->transform.SetPosition(spawnPos);

	// Box2D 물리 강체 좌표도 스폰 위치로 즉시 동기화 (0,0 겹침 방지)
	ColliderComponent* pCollider = pPlayerObj->GetComponent<ColliderComponent>();
	if (pCollider && b2Body_IsValid(pCollider->GetBodyId()))
	{
		b2Vec2 b2Pos = { PixelToMeter(spawnPos.x), PixelToMeter(spawnPos.y) };
		b2Body_SetTransform(pCollider->GetBodyId(), b2Pos, b2Rot_identity);
	}

	NetworkIdentity* netIdentity = pPlayerObj->GetComponent<NetworkIdentity>();
	if (!netIdentity)
	{
		netIdentity = pPlayerObj->AddComponent<NetworkIdentity>();
	}
	netIdentity->SetNetID(netId);
	netIdentity->SetLocalPlayer(isLocal);

	// SkillComponent 가져오기 (없으면 추가)
	SkillComponent* pSkillComp = pPlayerObj->GetComponent<SkillComponent>();
	if (!pSkillComp)
	{
		pSkillComp = pPlayerObj->AddComponent<SkillComponent>();
	}

	m_playerObjects[netId] = pPlayerObj;

	return pPlayerObj;
}

void InGameManager::SendClientReadyStatus(bool isReady)
{
	m_bIsMyReady = isReady;
	uint32 myNetID = NetworkManager::GetInstance()->GetMyNetID();

	ClientReadyReqPacket pk{};
	pk.header.type = PacketType::CLIENT_READY_REQ;
	pk.header.size = sizeof(ClientReadyReqPacket);
	pk.netId = myNetID;
	pk.isReady = isReady;

	NetworkManager::GetInstance()->SendPacket(&pk, sizeof(pk));
}

void InGameManager::SendHostStartSignal()
{
	if (!IsAllClientsReady())return;

	uint32 newSeed = RandomManager::GetInstance()->GenerateNewSeed();

	GameStartSignalPacket pk{};
	pk.header.type = PacketType::GAME_START_SIGNAL;
	pk.header.size = sizeof(GameStartSignalPacket);
	pk.randomSeed = newSeed;
	pk.countdown = 3.0f;

	NetworkManager::GetInstance()->SendPacket(&pk, sizeof(pk));

	this->m_countdownTimer = 3.0f;
	this->m_bIsCountDown = true;

	this->SortedPlayerCache();
}

bool InGameManager::IsAllClientsReady() const
{
	NetworkManager* net = NetworkManager::GetInstance();
	const auto& clients = net->GetConnectedClients();

	if (clients.empty())return true;

	for (const auto& [netID, info] : clients)
	{
		auto iter = m_clientReadyMap.find(netID);
		if (iter == m_clientReadyMap.end() || !iter->second)
		{
			return false;
		}
	}
	return true;
}
