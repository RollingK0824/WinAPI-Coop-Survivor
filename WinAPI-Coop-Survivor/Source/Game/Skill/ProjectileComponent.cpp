#include "Engine/Core/pch.h"
#include "ProjectileComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Physics/ColliderComponent.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Game/Monster/Monster.h"
#include "Game/Interface/IDamageable.h"

static ComponentRegistrar<ProjectileComponent> registrar(EngineKey::CustomComponent::ProjectileComponent.data());

ProjectileComponent::ProjectileComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
}

void ProjectileComponent::Init(const Vector2& dir, float speed, float damage, int32 penetration, float range, GameObject* attacker, const std::string& poolKey)
{
	m_direction = dir.GetNormalized();
	m_speed = speed;
	m_damage = damage;
	m_penetrationCount = penetration;
	m_range = range;
	m_traveledDistance = 0.0f;
	m_pAttacker = attacker;
	m_poolKey = poolKey;
	m_hitMonsterIDs.clear();
}

void ProjectileComponent::FixedUpdate(float fixedDt)
{
	if (!gameObject.IsActive()) return;

	Vector2 moveStep = m_direction * (m_speed * fixedDt);
	Vector2 curPos = transform.GetPosition();
	Vector2 newPos = curPos + moveStep;
	transform.SetPosition(newPos.x, newPos.y);

	m_traveledDistance += moveStep.Length();
	if (m_traveledDistance >= m_range)
	{
		gameObject.SetActive(false);
	}
}

void ProjectileComponent::OnCollision(ColliderComponent* pOtherCollider)
{
	if (!gameObject.IsActive() || pOtherCollider == nullptr) return;

	GameObject* pOtherObj = &pOtherCollider->gameObject;
	if (pOtherObj == nullptr || !pOtherObj->IsActive()) return;

	Monster* pMonster = pOtherObj->GetComponent<Monster>();
	if (pMonster != nullptr && !pMonster->IsDead())
	{
		uint32 monsterSeqID = pMonster->GetSpawnSeqID();
		if (m_hitMonsterIDs.find(monsterSeqID) != m_hitMonsterIDs.end())
		{
			return;
		}

		m_hitMonsterIDs.insert(monsterSeqID);
		pMonster->TakeDamage(m_damage, m_pAttacker.Get());

		m_penetrationCount--;
		if (m_penetrationCount <= 0)
		{
			gameObject.SetActive(false);
		}
	}
}