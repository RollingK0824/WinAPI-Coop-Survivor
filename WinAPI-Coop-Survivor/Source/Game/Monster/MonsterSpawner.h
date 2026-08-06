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

	void InitPool(size_t defaultCapacity = 300, size_t maxSize = 1000);
	Monster* SpawnMonster(MonsterSO* monsterData, const Vector2& spawnPos);
	void DespawnMonster(GameObject* pMonsterObj);

	void SetSpawningEnabled(bool enable) { m_isSpawningEnabled = enable; }
	void SetSpawnInterval(float interval) { m_spawnInterval = interval; }
	void SetMaxActiveMonsters(size_t count) { m_maxActiveMonsters = count; }
	void SetPrefabKey(const std::string& prefabKey) { m_prefabKey = prefabKey; }

	uint32 GetCurrentSpawnSeqID() const { return m_nextSpawnSeqID; }
	size_t GetActiveMonsterCount() const { return m_activeMonsterCount; }

private:
	Vector2 CalculateDeterministicSpawnPos();

private:
	uint32 m_nextSpawnSeqID = 0;
	bool m_isSpawningEnabled = true;

	std::string m_prefabKey = "TempMonster";
	float m_spawnInterval = 1.0f;
	float m_spawnTimer = 0.0f;
	int32 m_spawnCountPerWave = 3;
	size_t m_maxActiveMonsters = 1000;
	size_t m_activeMonsterCount = 0;

	float m_spawnRadiusMin = 700.0f;
	float m_spawnRadiusMax = 900.0f;

	ObserverPtr<MonsterSO> m_pDefaultMonsterSO;
};
