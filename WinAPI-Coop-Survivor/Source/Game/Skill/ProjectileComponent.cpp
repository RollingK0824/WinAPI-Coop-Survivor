#include "Engine/Core/pch.h"
#include "ProjectileComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Physics/ColliderComponent.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"
#include "Game/Monster/Monster.h"
#include "Game/Interface/IDamageable.h"
#include "SkillSO.h"

static ComponentRegistrar<ProjectileComponent> registrar(EngineKey::CustomComponent::ProjectileComponent.data());

ProjectileComponent::ProjectileComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeVariable("Speed", &m_speed);
	ExposeVariable("Damage", &m_damage);
	ExposeVariable("Range", &m_range);
	ExposeVariable("PenetrationCount", &m_penetrationCount);
}

void ProjectileComponent::Init(const Vector2& dir, const SkillLevelData& data, const SkillSO* pSO, GameObject* attacker, const std::string& poolKey)
{
	m_direction = dir.GetNormalized();
	m_speed = data.speed;
	m_damage = data.damage;
	m_penetrationCount = data.penetrationCount;
	m_range = data.range;
	m_traveledDistance = 0.0f;
	m_pAttacker = attacker;
	m_poolKey = poolKey;
	m_effectKey = pSO ? pSO->GetEffectKey() : "";
	m_hitInstanceIDs.clear();

	// 방향에 따른 회전각 설정
	float angleRad = atan2f(m_direction.y, m_direction.x);
	transform.SetRotation(angleRad);

	// 동적 스프라이트 주입
	if (auto renderer = gameObject.GetComponent<SpriteRendererComponent>())
	{
		if (pSO && !pSO->GetSpriteKey().empty())
		{
			renderer->SetSpriteKey(pSO->GetSpriteKey());
		}
	}

	ColliderComponent* pCol = gameObject.GetComponent<ColliderComponent>();
	if (!pCol)
	{
		std::cout << "[ProjectileComponent] Error: Projectile GameObject missing ColliderComponent!" << std::endl;
		return;
	}

	b2BodyId bodyId = pCol->GetBodyId();
	if (b2Body_IsValid(bodyId))
	{
		Vector2 spawnPos = transform.GetPosition();
		b2Vec2 b2Pos = { PixelToMeter(spawnPos.x), PixelToMeter(spawnPos.y) };
		b2Rot b2Rot = b2MakeRot(angleRad);
		b2Body_SetTransform(bodyId, b2Pos, b2Rot);

		Vector2 velocity = m_direction * m_speed;
		b2Vec2 b2Vel = { PixelToMeter(velocity.x), PixelToMeter(velocity.y) };
		b2Body_SetLinearVelocity(bodyId, b2Vel);
	}
}

void ProjectileComponent::FixedUpdate(float fixedDt)
{
	if (!gameObject.IsActive()) return;

	float stepLength = m_speed * fixedDt;
	m_traveledDistance += stepLength;

	if (m_traveledDistance >= m_range)
	{
		PoolManager::GetInstance()->Despawn<GameObject>(m_poolKey, &gameObject);
	}
}

void ProjectileComponent::OnCollision(ColliderComponent* pOtherCollider)
{
	if (!gameObject.IsActive() || pOtherCollider == nullptr) return;

	GameObject* pOtherObj = &pOtherCollider->gameObject;
	if (pOtherObj == nullptr || !pOtherObj->IsActive()) return;

	IDamageable* pDamageable = pOtherObj->GetComponent<IDamageable>();
	if (pDamageable != nullptr && !pDamageable->IsDead())
	{
		uint64 instanceID = pOtherObj->GetInstanceID();
		if (m_hitInstanceIDs.find(instanceID) != m_hitInstanceIDs.end())
		{
			return;
		}

		m_hitInstanceIDs.insert(instanceID);
		pDamageable->TakeDamage(m_damage, m_pAttacker.Get());

		if (!m_effectKey.empty())
		{
			GameObject* pVFX = PoolManager::GetInstance()->Spawn<GameObject>(m_effectKey);
			if (pVFX)
			{
				pVFX->transform.SetPosition(transform.GetPosition().x, transform.GetPosition().y);
			}
		}

		// 타격 관통 처리 (-1은 무한 관통, >0 일 경우 차감 후 0 이하 시 Despawn)
		if (m_penetrationCount > 0)
		{
			m_penetrationCount--;
			if (m_penetrationCount <= 0)
			{
				PoolManager::GetInstance()->Despawn<GameObject>(m_poolKey, &gameObject);
			}
		}
	}
}