#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/ObserverPtr.h"

class GameObject;
class Monster;
class MonsterSO;

class MonsterSpawner : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(MonsterSpawner)

	MonsterSpawner(GameObject* owner, TransformComponent* transform);
	virtual ~MonsterSpawner() override = default;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::MonsterSpawner;
	}

	virtual void Start() override;
	virtual void FixedUpdate(float fixedDt) override;

	void StartSpawning();
	void StopSpawning();

	void InitPool(size_t defaultCapacity = 500, size_t maxSize = 1500);
	Monster* SpawnMonster(MonsterSO* monsterData, const Vector2& spawnPos);
	Monster* SpawnMonsterClient(uint16 netID, uint32 monsterAssetID, const Vector2& spawnPos);
	void DespawnMonster(GameObject* pMonsterObj);
	Monster* GetMonsterByNetID(uint16 netID);
	void DespawnMonsterByNetID(uint16 netID);

	void SetSpawningEnabled(bool enable) { m_isSpawningEnabled = enable; }
	void SetSpawnInterval(float interval) { m_spawnInterval = interval; }
	void SetMaxActiveMonsters(size_t count) { m_maxActiveMonsters = count; }
	void SetPrefabKey(const std::string& prefabKey) { m_prefabKey = prefabKey; }
	void SetMonsterAssetIDs(const std::vector<uint32>& ids) { m_spawnMonsterAssetIDs = ids; RefreshMonsterSOs(); }

	uint32 GetCurrentSpawnSeqID() const { return m_nextSpawnSeqID; }
	size_t GetActiveMonsterCount() const { return m_activeMonsterCount; }

private:
	Vector2 CalculateDeterministicSpawnPos();
	void RefreshMonsterSOs();

private:
	uint32 m_nextSpawnSeqID = 0;
	uint16 m_nextMonsterNetID = 2000; // Player NetID(1~999)와 충돌 방지
	bool m_isSpawningEnabled = false;

	std::unordered_map<uint16, Monster*> m_activeMonsterMap;
	float m_snapshotTimer = 0.0f;
	const float m_snapshotInterval = 0.066f; // 15Hz

	std::string m_prefabKey = "GenericMonster";
	float m_spawnInterval = 1.0f;
	float m_spawnTimer = 0.0f;
	int32 m_spawnCountPerWave = 3;
	size_t m_maxActiveMonsters = 1000;
	size_t m_activeMonsterCount = 0;

	float m_spawnRadiusMin = 700.0f;
	float m_spawnRadiusMax = 900.0f;

	std::vector<uint32> m_spawnMonsterAssetIDs;
	std::vector<ObserverPtr<MonsterSO>> m_spawnMonsterSOs;
	ObserverPtr<MonsterSO> m_pDefaultMonsterSO;
};
