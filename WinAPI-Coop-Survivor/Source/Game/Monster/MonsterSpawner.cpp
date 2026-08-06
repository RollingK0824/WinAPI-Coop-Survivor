#include "Engine/Core/pch.h"
#include "Game/Monster/MonsterSpawner.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/CircleCollider.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/DataManager.h"
#include "Engine/Manager/RandomManager.h"
#include "Engine/Manager/PrefabManager.h"
#include "Game/Monster/Monster.h"
#include "Game/Data/MonsterSO.h"
#include "Game/Player/Player.h"

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
				for (auto* comp : pObj->GetComponents())
				{
					if (comp)
					{
						comp->SetEnabled(false);
						if (auto* collider = dynamic_cast<ColliderComponent*>(comp))
						{
							if (b2Body_IsValid(collider->GetBodyId()))
							{
								b2Body_Disable(collider->GetBodyId());
							}
						}
					}
				}
			}
			return pObj;
		},
		[](GameObject* pObj) {
			if (pObj)
			{
				pObj->SetActive(true);
				for (auto* comp : pObj->GetComponents())
				{
					if (comp)
					{
						comp->SetEnabled(true);
						if (auto* collider = dynamic_cast<ColliderComponent*>(comp))
						{
							if (b2Body_IsValid(collider->GetBodyId()))
							{
								b2Body_Enable(collider->GetBodyId());
							}
						}
					}
				}
			}
		},
		[](GameObject* pObj) {
			if (pObj)
			{
				pObj->SetActive(false);
				for (auto* comp : pObj->GetComponents())
				{
					if (comp)
					{
						comp->SetEnabled(false);
						if (auto* collider = dynamic_cast<ColliderComponent*>(comp))
						{
							if (b2Body_IsValid(collider->GetBodyId()))
							{
								b2Body_Disable(collider->GetBodyId());
							}
						}
					}
				}
			}
		},
		nullptr,
		defaultCapacity,
		maxSize
	);
}

void MonsterSpawner::FixedUpdate(float fixedDt)
{
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

Vector2 MonsterSpawner::CalculateDeterministicSpawnPos()
{
	Scene* pScene = gameObject.GetOwnerScene();
	if (!pScene) return { 0.0f, 0.0f };

	std::vector<GameObject*> players;
	const auto& sceneObjects = pScene->GetGameObjects();
	for (const auto& pObj : sceneObjects)
	{
		if (pObj && pObj->IsActive() && pObj->GetComponent<Player>())
		{
			players.push_back(pObj);
		}
	}

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
	for (auto* comp : pMonsterObj->GetComponents())
	{
		if (comp)
		{
			comp->SetEnabled(true);
			if (auto* collider = dynamic_cast<ColliderComponent*>(comp))
			{
				if (b2Body_IsValid(collider->GetBodyId()))
				{
					b2Body_Enable(collider->GetBodyId());
				}
			}
		}
	}

	Monster* pMonsterComp = pMonsterObj->GetComponent<Monster>();
	if (!pMonsterComp) return nullptr;

	uint32 seqId = m_nextSpawnSeqID++;
	pMonsterComp->Init(seqId, monsterData, spawnPos, this);

	m_activeMonsterCount++;
	return pMonsterComp;
}

void MonsterSpawner::DespawnMonster(GameObject* pMonsterObj)
{
	if (!pMonsterObj) return;

	pMonsterObj->SetActive(false);
	for (auto* comp : pMonsterObj->GetComponents())
	{
		if (comp)
		{
			comp->SetEnabled(false);
			if (auto* collider = dynamic_cast<ColliderComponent*>(comp))
			{
				if (b2Body_IsValid(collider->GetBodyId()))
				{
					b2Body_Disable(collider->GetBodyId());
				}
			}
		}
	}

	PoolManager::GetInstance()->Despawn<GameObject>(m_prefabKey, pMonsterObj);
	if (m_activeMonsterCount > 0)
	{
		m_activeMonsterCount--;
	}
}
