#include "Engine/Core/pch.h"
#include "AuraComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Game/Monster/Monster.h"

static ComponentRegistrar<AuraComponent> registrar(EngineKey::CustomComponent::AuraComponent.data());

AuraComponent::AuraComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
}

void AuraComponent::Init(float damage, float range, float duration, GameObject* pCaster)
{
	m_damage = damage;
	m_range = range;
	m_duration = duration;
	m_pCaster = pCaster;
	m_lifeTimer = 0.0f;
	m_tickTimer = 0.0f;
}

void AuraComponent::FixedUpdate(float fixedDt)
{
	if (!gameObject.IsActive()) return;

	if (m_pCaster.IsValid() && m_pCaster->IsActive())
	{
		Vector2 casterPos = m_pCaster->transform.GetPosition();
		transform.SetPosition(casterPos.x, casterPos.y);
	}

	m_tickTimer += fixedDt;
	if (m_tickTimer >= m_tickInterval)
	{
		m_tickTimer = 0.0f;
		ApplyAreaDamage();
	}

	if (m_duration > 0.0f)
	{
		m_lifeTimer += fixedDt;
		if (m_lifeTimer >= m_duration)
		{
			gameObject.SetActive(false);
		}
	}
}

void AuraComponent::ApplyAreaDamage()
{
	Scene* pScene = SceneManager::GetInstance()->GetActiveScene();
	if (!pScene) return;

	Vector2 myPos = transform.GetPosition();
	const auto& objects = pScene->GetGameObjects();

	for (GameObject* pObj : objects)
	{
		if (!pObj || !pObj->IsActive() || pObj->IsDead()) continue;

		Monster* pMonster = pObj->GetComponent<Monster>();
		if (pMonster && !pMonster->IsDead())
		{
			Vector2 monsterPos = pObj->transform.GetPosition();
			float distSq = (monsterPos - myPos).LengthSquared();
			if (distSq <= (m_range * m_range))
			{
				pMonster->TakeDamage(m_damage, m_pCaster.Get());
			}
		}
	}
}
