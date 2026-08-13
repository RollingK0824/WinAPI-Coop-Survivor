#include "Engine/Core/pch.h"
#include "Game/Monster/MonsterSpawner.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/TimeManager.h"
#include "Engine/Manager/DataManager.h"
#include "Engine/Manager/RandomManager.h"
#include "Engine/Manager/PrefabManager.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Network/NetPacket.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/CircleCollider.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"
#include "Engine/Framework/Components/Network/NetworkIdentity.h"
#include "Game/Monster/Monster.h"
#include "Game/Monster/MonsterSO.h"
#include "Game/Player/Player.h"
#include "Game/Manager/InGameManager.h"

static ComponentRegistrar<MonsterSpawner> registrar(EngineKey::CustomComponent::MonsterSpawner.data());

MonsterSpawner::MonsterSpawner(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeVariable("PrefabKey", &m_prefabKey);
	ExposeVariable("IsSpawningEnabled", &m_isSpawningEnabled);
	ExposeVariable("SpawnInterval", &m_spawnInterval);
	ExposeVariable("SpawnCountPerWave", &m_spawnCountPerWave);
	ExposeVariable("SpawnRadiusMin", &m_spawnRadiusMin);
	ExposeVariable("SpawnRadiusMax", &m_spawnRadiusMax);
}

void MonsterSpawner::Start()
{
	InitPool(300, 1000);
}

void MonsterSpawner::InitPool(size_t defaultCapacity, size_t maxSize)
{
	Scene* pScene = gameObject.GetOwnerScene();
	if (!pScene) return;

	PoolManager::GetInstance()->CreatePool<GameObject>(
		m_prefabKey,
		[this, pScene]() -> GameObject* {
			GameObject* pObj = PrefabManager::GetInstance()->Instantiate(m_prefabKey, pScene);
			if (pObj)
			{
				pObj->SetActive(false);
			}
			return pObj;
		},
		[](GameObject* pObj) {
			if (pObj)
			{
				pObj->SetActive(true);
			}
		},
		[](GameObject* pObj) {
			if (pObj)
			{
				pObj->SetActive(false);
			}
		},
		nullptr,
		defaultCapacity,
		maxSize
	);
}

void MonsterSpawner::FixedUpdate(float fixedDt)
{
	NetRole role = NetworkManager::GetInstance()->GetRole();

	// Client는 Host 스냅샷 전용이므로 독자적인 웨이브 스폰을 실행하지 않음!
	if (role == NetRole::CLIENT) return;

	// Host 전용: 15Hz (66ms) 주기로 Client별 공간 컬링 스냅샷 패킷 발송
	if (role == NetRole::HOST)
	{
		m_snapshotTimer += fixedDt;
		if (m_snapshotTimer >= m_snapshotInterval)
		{
			m_snapshotTimer = 0.0f;

			const auto& connectedClients = NetworkManager::GetInstance()->GetConnectedClients();
			Scene* pScene = gameObject.GetOwnerScene();

			for (const auto& [clientNetID, clientInfo] : connectedClients)
			{
				Vector2 clientPos = { 0.0f, 0.0f };
				GameObject* clientPlayerObj = NetworkManager::GetInstance()->GetNetworkObject(clientNetID);
				if (clientPlayerObj && clientPlayerObj->IsActive())
				{
					clientPos = clientPlayerObj->transform.GetPosition();
				}
				else if (pScene)
				{
					for (auto* obj : pScene->GetGameObjects())
					{
						if (obj && obj->IsActive())
						{
							auto* netComp = obj->GetComponent<NetworkIdentity>();
							if (netComp && netComp->GetNetID() == clientNetID)
							{
								clientPos = obj->transform.GetPosition();
								break;
							}
						}
					}
				}

				std::vector<MonsterSnapshotData> culledMonsters;
				culledMonsters.reserve(m_activeMonsterMap.size());

				for (auto& [netID, pMonster] : m_activeMonsterMap)
				{
					if (!pMonster || pMonster->IsDead() || !pMonster->gameObject.IsActive()) continue;

					Vector2 monsterPos = pMonster->transform.GetPosition();
					float distSq = Vector2::DistanceSquared(monsterPos, clientPos);

					// 화면 외곽(1920x1080 반경) 950px Culling
					if (distSq <= 950.0f * 950.0f)
					{
						culledMonsters.push_back({ netID, monsterPos });
					}
				}

				if (culledMonsters.empty()) continue;

				// UDP MTU (1472B) 제한 준수를 위해 100마리 단위 청크 분할 발송
				const size_t MAX_PER_PACKET = 100;
				size_t totalMonsters = culledMonsters.size();
				size_t offset = 0;

				while (offset < totalMonsters)
				{
					size_t chunkSize = (std::min)(MAX_PER_PACKET, totalMonsters - offset);
					size_t packetSize = sizeof(MonsterSnapshotPacket) + (chunkSize - 1) * sizeof(MonsterSnapshotData);
					std::vector<char> buffer(packetSize);

					auto* packet = reinterpret_cast<MonsterSnapshotPacket*>(buffer.data());
					packet->header.type = PacketType::MONSTER_SNAPSHOT;
					packet->header.size = static_cast<uint16>(packetSize);
					packet->monsterCount = static_cast<uint16>(chunkSize);

					std::memcpy(packet->monsters, &culledMonsters[offset], chunkSize * sizeof(MonsterSnapshotData));

					NetworkManager::GetInstance()->SendPacket(packet, static_cast<int>(packetSize), &clientInfo.address);
					offset += chunkSize;
				}
			}
		}
	}

	if (!m_isSpawningEnabled) return;

	m_spawnTimer += fixedDt;
	if (m_spawnTimer >= m_spawnInterval)
	{
		m_spawnTimer = 0.0f;

		if (m_activeMonsterCount >= m_maxActiveMonsters)
			return;

		for (int i = 0; i < m_spawnCountPerWave; ++i)
		{
			Vector2 spawnPos = CalculateDeterministicSpawnPos();
			SpawnMonster(m_pDefaultMonsterSO.Get(), spawnPos);
		}
	}
}

void MonsterSpawner::StartSpawning()
{
	m_spawnTimer = 0.0f;
	m_isSpawningEnabled = true;
}

void MonsterSpawner::StopSpawning()
{
	m_isSpawningEnabled = false;
}

Vector2 MonsterSpawner::CalculateDeterministicSpawnPos()
{
	InGameManager* inGameMgr = InGameManager::GetInstance();
	if (!inGameMgr) return { 0.0f, 0.0f };

	const auto& players = inGameMgr->GetPlayers();
	Vector2 centerPos = { 0.0f, 0.0f };
	if (!players.empty())
	{
		int idx = RandomManager::GetInstance()->GetSharedRandomInt(0, static_cast<int>(players.size()) - 1);
		centerPos = players[idx]->transform.GetPosition();
	}

	float angleDegree = RandomManager::GetInstance()->GetSharedRandomFloat(0.0f, 360.0f);
	float radian = angleDegree * (3.14159265f / 180.0f);
	float radius = RandomManager::GetInstance()->GetSharedRandomFloat(m_spawnRadiusMin, m_spawnRadiusMax);

	Vector2 offset = { std::cos(radian) * radius, std::sin(radian) * radius };
	return centerPos + offset;
}

Monster* MonsterSpawner::SpawnMonster(MonsterSO* monsterData, const Vector2& spawnPos)
{
	GameObject* pMonsterObj = PoolManager::GetInstance()->Spawn<GameObject>(m_prefabKey);
	if (!pMonsterObj) return nullptr;

	pMonsterObj->SetActive(true);

	Monster* pMonsterComp = pMonsterObj->GetComponent<Monster>();
	if (!pMonsterComp) return nullptr;

	uint32 seqId = m_nextSpawnSeqID++;
	uint16 netID = m_nextMonsterNetID++;
	if (m_nextMonsterNetID >= 60000)
	{
		m_nextMonsterNetID = 2000;
	}
	pMonsterComp->SetNetID(netID);
	pMonsterComp->Init(seqId, monsterData, spawnPos, this);

	m_activeMonsterMap[netID] = pMonsterComp;
	m_activeMonsterCount++;
	return pMonsterComp;
}

Monster* MonsterSpawner::SpawnMonsterClient(uint16 netID, const Vector2& spawnPos)
{
	GameObject* pMonsterObj = PoolManager::GetInstance()->Spawn<GameObject>(m_prefabKey);
	if (!pMonsterObj) return nullptr;

	pMonsterObj->SetActive(true);

	Monster* pMonsterComp = pMonsterObj->GetComponent<Monster>();
	if (!pMonsterComp) return nullptr;

	pMonsterComp->SetNetID(netID);
	pMonsterComp->Init(0, m_pDefaultMonsterSO.Get(), spawnPos, this);

	m_activeMonsterMap[netID] = pMonsterComp;
	m_activeMonsterCount++;
	return pMonsterComp;
}

void MonsterSpawner::DespawnMonster(GameObject* pMonsterObj)
{
	if (!pMonsterObj) return;

	Monster* pMonsterComp = pMonsterObj->GetComponent<Monster>();
	if (pMonsterComp)
	{
		uint16 netID = pMonsterComp->GetNetID();
		m_activeMonsterMap.erase(netID);
		// NetworkIdentity가 GameObject와 함께 소멸하므로 별도 보간 정리 불필요
	}

	pMonsterObj->SetActive(false);
	PoolManager::GetInstance()->Despawn<GameObject>(m_prefabKey, pMonsterObj);
	if (m_activeMonsterCount > 0)
	{
		m_activeMonsterCount--;
	}
}

Monster* MonsterSpawner::GetMonsterByNetID(uint16 netID)
{
	auto it = m_activeMonsterMap.find(netID);
	if (it != m_activeMonsterMap.end())
	{
		return it->second;
	}
	return nullptr;
}

void MonsterSpawner::DespawnMonsterByNetID(uint16 netID)
{
	Monster* pMonster = GetMonsterByNetID(netID);
	if (pMonster)
	{
		DespawnMonster(&pMonster->gameObject);
	}
}
