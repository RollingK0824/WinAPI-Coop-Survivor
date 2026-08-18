#include "Engine/Core/pch.h"
#include "AoEComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Physics/PhysicsManager.h"
#include "Engine/Framework/Components/Physics/ColliderComponent.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"
#include "Engine/Framework/GameObject.h"
#include "Game/Interface/IDamageable.h"
#include "SkillSO.h"

static ComponentRegistrar<AoEComponent> registrar(EngineKey::CustomComponent::AoEComponent.data());

AoEComponent::AoEComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
}

void AoEComponent::Init(const SkillLevelData& data, const SkillSO* pSO, GameObject* attacker, const std::string& poolKey)
{
	m_damage = data.damage;
	m_range = data.range;
	m_duration = (data.duration > 0.0f) ? data.duration : 0.15f;
	m_tickInterval = (data.cooldown > 0.0f && data.duration > data.cooldown) ? data.cooldown : 0.0f;
	m_tickTimer = m_tickInterval;
	m_lifeTimer = 0.0f;
	m_hasAppliedDamage = false;
	m_pAttacker = attacker;
	m_poolKey = poolKey;

	if (auto renderer = gameObject.GetComponent<SpriteRendererComponent>())
	{
		if (pSO && !pSO->GetSpriteKey().empty())
		{
			renderer->SetSpriteKey(pSO->GetSpriteKey());
		}
	}
}

void AoEComponent::FixedUpdate(float fixedDt)
{
	if (!gameObject.IsActive()) return;

	if (m_tickInterval <= 0.0f)
	{
		if (!m_hasAppliedDamage)
		{
			m_hasAppliedDamage = true;
			ApplyExplosionDamage();
		}
	}
	else
	{
		m_tickTimer += fixedDt;
		if (m_tickTimer >= m_tickInterval)
		{
			m_tickTimer -= m_tickInterval;
			ApplyExplosionDamage();
		}
	}

	m_lifeTimer += fixedDt;
	if (m_lifeTimer >= m_duration)
	{
		PoolManager::GetInstance()->Despawn<GameObject>(m_poolKey, &gameObject);
	}
}

void AoEComponent::ApplyExplosionDamage()
{
	Vector2 centerPos = transform.GetPosition();
	auto colliders = PhysicsManager::GetInstance()->OverlapAABB(centerPos, m_range, PhysicsLayer::Monster);

	for (ColliderComponent* pCol : colliders)
	{
		if (!pCol || !pCol->IsEnabled() || !pCol->gameObject.IsActive()) continue;

		IDamageable* pDamageable = pCol->gameObject.GetComponent<IDamageable>();
		if (pDamageable && !pDamageable->IsDead())
		{
			Vector2 targetPos = pCol->transform.GetPosition();
			float distSq = (targetPos - centerPos).LengthSquared();
			if (distSq <= (m_range * m_range))
			{
				pDamageable->TakeDamage(m_damage, m_pAttacker.Get());
			}
		}
	}
}
