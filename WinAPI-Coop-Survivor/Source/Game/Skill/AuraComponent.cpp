#include "Engine/Core/pch.h"
#include "AuraComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Physics/PhysicsManager.h"
#include "Engine/Framework/Components/Physics/ColliderComponent.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"
#include "Engine/Framework/GameObject.h"
#include "Game/Interface/IDamageable.h"
#include "SkillSO.h"

static ComponentRegistrar<AuraComponent> registrar(EngineKey::CustomComponent::AuraComponent.data());

AuraComponent::AuraComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
}

void AuraComponent::Init(const SkillLevelData& data, const SkillSO* pSO, GameObject* pCaster, const std::string& poolKey)
{
	m_damage = data.damage;
	m_range = data.range;
	m_duration = data.duration;
	m_tickInterval = (data.cooldown > 0.0f) ? data.cooldown : 0.5f;
	m_tickTimer = m_tickInterval; // 즉시 첫 틱 발동
	m_lifeTimer = 0.0f;
	m_pCaster = pCaster;
	m_poolKey = poolKey;

	if (auto renderer = gameObject.GetComponent<SpriteRendererComponent>())
	{
		if (pSO && !pSO->GetSpriteKey().empty())
		{
			renderer->SetSpriteKey(pSO->GetSpriteKey());
		}
	}
}

void AuraComponent::FixedUpdate(float fixedDt)
{
	if (!gameObject.IsActive()) return;

	if (m_pCaster.IsValid())
	{
		transform.SetPosition(m_pCaster->transform.GetPosition().x, m_pCaster->transform.GetPosition().y);
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
			PoolManager::GetInstance()->Despawn<GameObject>(m_poolKey, &gameObject);
		}
	}
}

void AuraComponent::ApplyAreaDamage()
{
	Vector2 myPos = transform.GetPosition();
	auto colliders = PhysicsManager::GetInstance()->OverlapAABB(myPos, m_range, PhysicsLayer::Monster);

	for (ColliderComponent* pCol : colliders)
	{
		if (!pCol || !pCol->IsEnabled() || !pCol->gameObject.IsActive()) continue;

		IDamageable* pDamageable = pCol->gameObject.GetComponent<IDamageable>();
		if (pDamageable && !pDamageable->IsDead())
		{
			Vector2 monsterPos = pCol->transform.GetPosition();
			float distSq = (monsterPos - myPos).LengthSquared();
			if (distSq <= (m_range * m_range))
			{
				pDamageable->TakeDamage(m_damage, m_pCaster.Get());
			}
		}
	}
}
