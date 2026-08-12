#include "Engine/Core/pch.h"
#include "AoEComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Game/Monster/Monster.h"

static ComponentRegistrar<AoEComponent> registrar(EngineKey::CustomComponent::AoEComponent.data());

AoEComponent::AoEComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
}

void AoEComponent::Init(float damage, float range, float duration, const Vector2& spawnPos, GameObject* pAttacker)
{
	m_damage = damage;
	m_range = range;
	m_duration = (std::max)(0.1f, duration);
	m_lifeTimer = 0.0f;
	m_hasAppliedDamage = false;
	m_pAttacker = pAttacker;
	transform.SetPosition(spawnPos.x, spawnPos.y);
}

void AoEComponent::FixedUpdate(float fixedDt)
{
	if (!gameObject.IsActive()) return;

	if (!m_hasAppliedDamage)
	{
		m_hasAppliedDamage = true;
		ApplyExplosionDamage();
	}

	m_lifeTimer += fixedDt;
	if (m_lifeTimer >= m_duration)
	{
		gameObject.SetActive(false);
	}
}

void AoEComponent::ApplyExplosionDamage()
{
	Scene* pScene = SceneManager::GetInstance()->GetActiveScene();
	if (!pScene) return;

	Vector2 centerPos = transform.GetPosition();
	const auto& objects = pScene->GetGameObjects();

	for (GameObject* pObj : objects)
	{
		if (!pObj || !pObj->IsActive() || pObj->IsDead()) continue;

		Monster* pMonster = pObj->GetComponent<Monster>();
		if (pMonster && !pMonster->IsDead())
		{
			Vector2 monsterPos = pObj->transform.GetPosition();
			float distSq = (monsterPos - centerPos).LengthSquared();
			if (distSq <= (m_range * m_range))
			{
				pMonster->TakeDamage(m_damage, m_pAttacker.Get());
			}
		}
	}
}
