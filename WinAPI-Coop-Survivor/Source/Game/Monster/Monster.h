#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Game/Interface/IDamageable.h"
#include "Engine/Core/ObserverPtr.h"

class GameObject;
class MonsterSO;
class MonsterSpawner;
class CircleCollider;

enum class EMonsterState
{
	Spawn,
	Chase,
	Attack,
	Dead
};

class Monster : public ScriptComponent, public IDamageable
{
public:
	CLONEABLE_COMPONENT(Monster)

	Monster(GameObject* owner, TransformComponent* transform);
	virtual ~Monster() override = default;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::Monster;
	}

	virtual void Start() override;
	virtual void FixedUpdate(float fixedDt) override;

	void Init(uint32 spawnSeqId, MonsterSO* monsterData, const Vector2& spawnPos, MonsterSpawner* spawner = nullptr);
	void SetTargetPlayer(GameObject* pTargetObj);
	void SetSpawner(MonsterSpawner* spawner);

	virtual void TakeDamage(float damage, GameObject* pAttacker = nullptr) override;
	virtual bool IsDead() const override { return m_state == EMonsterState::Dead; }

	uint32 GetSpawnSeqID() const { return m_spawnSeqID; }
	float GetCurrentHP() const { return m_currentHP; }
	float GetMaxHP() const { return m_maxHP; }
	EMonsterState GetState() const { return m_state; }

protected:
	virtual void UpdateAI(float fixedDt);
	virtual void UpdateBehaviour(float fixedDt);
	virtual void OnAttack();
	virtual void OnDie();

private:
	void UpdateTargetSearch(float fixedDt);
	void MoveTowardsTarget(float fixedDt);

private:
	uint32 m_monsterAssetID = 0;
	ObserverPtr<MonsterSO> m_pMonsterSO;

	uint32 m_spawnSeqID = 0;
	EMonsterState m_state = EMonsterState::Spawn;

	float m_currentHP = 100.0f;
	float m_maxHP = 100.0f;
	float m_moveSpeed = 120.0f;
	float m_attackDamage = 10.0f;
	float m_attackRange = 40.0f;
	float m_attackCooldown = 1.0f;
	float m_attackTimer = 0.0f;
	int32 m_expAmount = 10;

	ObserverPtr<GameObject> m_targetPlayer;
	float m_targetSearchInterval = 0.5f;
	float m_targetSearchTimer = 0.0f;
	float m_targetMargin = 100.0f;

	ObserverPtr<CircleCollider> m_pCollider;
	ObserverPtr<MonsterSpawner> m_pSpawner;
};
