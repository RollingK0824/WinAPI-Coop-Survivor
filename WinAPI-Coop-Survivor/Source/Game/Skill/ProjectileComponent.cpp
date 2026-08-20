#include "Engine/Core/pch.h"
#include "ProjectileComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Physics/ColliderComponent.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"
#include "Engine/Framework/Components/Render/AnimatorComponent.h"
#include "AoEComponent.h"
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
	m_duration = data.duration;
	m_traveledDistance = 0.0f;
	m_lifeTimer = 0.0f;
	m_pAttacker = attacker;
	m_poolKey = poolKey;
	m_effectKey = pSO ? pSO->GetEffectKey() : "";
	m_hitCooldowns.clear();

	// OnHit Explosion configuration
	if (pSO && !pSO->GetOnHitAnimKey().empty())
	{
		m_onHitAnimKey = pSO->GetOnHitAnimKey();
		m_onHitRadius = (data.onHitRadius > 0.0f) ? data.onHitRadius : (pSO->GetOnHitRadius() > 0.0f ? pSO->GetOnHitRadius() : 75.0f);
		m_onHitDamageRatio = pSO->GetOnHitDamageRatio();
		m_onHitDuration = pSO->GetOnHitDuration();
	}
	else
	{
		m_onHitAnimKey = "";
	}

	// Orbital vs Linear
	if (data.aimType == EAimType::Orbital)
	{
		m_isOrbital = true;
		m_orbitRadius = (data.range > 0.0f) ? data.range : 100.0f;
		// Speed treated as angular speed in rad/s (if > 10.0, assume deg/s)
		m_orbitSpeed = (data.speed > 10.0f) ? (data.speed * (3.14159265f / 180.0f)) : ((data.speed > 0.0f) ? data.speed : 3.5f);
		m_orbitAngle = atan2f(dir.y, dir.x);
		m_duration = (data.duration > 0.0f) ? data.duration : 4.0f;
		m_penetrationCount = -1; // Piercing during orbit

		if (m_pAttacker.IsValid())
		{
			Vector2 center = m_pAttacker->transform.GetPosition();
			Vector2 pos = center + Vector2{ cosf(m_orbitAngle), sinf(m_orbitAngle) } * m_orbitRadius;
			float rotDegree = RadianToDegree(m_orbitAngle) + 90.0f;
			transform.SetPosition(pos.x, pos.y);
			transform.SetRotation(rotDegree);
		}
	}
	else
	{
		m_isOrbital = false;
		float angleRad = atan2f(m_direction.y, m_direction.x);
		float spriteOffsetAngle = data.fixedAngleDeg;
		if (spriteOffsetAngle == 0.0f && pSO && (pSO->GetSkillID() == 304 || pSO->GetAssetName() == "Flame"))
		{
			spriteOffsetAngle = 90.0f;
		}

		float angleDeg = RadianToDegree(angleRad) + spriteOffsetAngle;
		transform.SetRotation(angleDeg);
	}

	// Visuals: Animation or Sprite
	if (pSO && !pSO->GetAnimClipKey().empty())
	{
		AnimatorComponent* pAnim = gameObject.GetComponent<AnimatorComponent>();
		if (!pAnim)
		{
			pAnim = gameObject.AddComponent<AnimatorComponent>();
		}
		if (pAnim)
		{
			std::wstring wKey(pSO->GetAnimClipKey().begin(), pSO->GetAnimClipKey().end());
			pAnim->Play(wKey, true);
		}
	}
	else
	{
		if (AnimatorComponent* pAnim = gameObject.GetComponent<AnimatorComponent>())
		{
			pAnim->Stop();
		}
		if (auto renderer = gameObject.GetComponent<SpriteRendererComponent>())
		{
			if (pSO && !pSO->GetSpriteKey().empty())
			{
				renderer->SetSpriteKey(pSO->GetSpriteKey());
			}
		}
	}

	// Physics body setup
	ColliderComponent* pCol = gameObject.GetComponent<ColliderComponent>();
	if (pCol)
	{
		b2BodyId bodyId = pCol->GetBodyId();
		if (b2Body_IsValid(bodyId))
		{
			Vector2 spawnPos = transform.GetPosition();
			float rotDeg = transform.GetLocalRotation();
			b2Vec2 b2Pos = { PixelToMeter(spawnPos.x), PixelToMeter(spawnPos.y) };
			b2Rot b2Rot = b2MakeRot(DegreeToRadian(rotDeg));
			b2Body_SetTransform(bodyId, b2Pos, b2Rot);

			if (m_isOrbital)
			{
				b2Body_SetLinearVelocity(bodyId, { 0.0f, 0.0f });
			}
			else
			{
				Vector2 velocity = m_direction * m_speed;
				b2Vec2 b2Vel = { PixelToMeter(velocity.x), PixelToMeter(velocity.y) };
				b2Body_SetLinearVelocity(bodyId, b2Vel);
			}
		}
	}
}

void ProjectileComponent::FixedUpdate(float fixedDt)
{
	if (!gameObject.IsActive()) return;

	// Cooldown timer update
	for (auto it = m_hitCooldowns.begin(); it != m_hitCooldowns.end(); )
	{
		it->second -= fixedDt;
		if (it->second <= 0.0f)
		{
			it = m_hitCooldowns.erase(it);
		}
		else
		{
			++it;
		}
	}

	if (m_isOrbital)
	{
		m_orbitAngle += m_orbitSpeed * fixedDt;
		if (m_pAttacker.IsValid())
		{
			Vector2 center = m_pAttacker->transform.GetPosition();
			Vector2 pos = center + Vector2{ cosf(m_orbitAngle), sinf(m_orbitAngle) } * m_orbitRadius;
			float rotDegree = RadianToDegree(m_orbitAngle) + 90.0f;
			float rotRadian = m_orbitAngle + DegreeToRadian(90.0f);

			transform.SetPosition(pos.x, pos.y);
			transform.SetRotation(rotDegree);

			if (ColliderComponent* pCol = gameObject.GetComponent<ColliderComponent>())
			{
				b2BodyId bodyId = pCol->GetBodyId();
				if (b2Body_IsValid(bodyId))
				{
					b2Vec2 b2Pos = { PixelToMeter(pos.x), PixelToMeter(pos.y) };
					b2Rot b2Rot = b2MakeRot(rotRadian);
					b2Body_SetTransform(bodyId, b2Pos, b2Rot);
					b2Body_SetLinearVelocity(bodyId, { 0.0f, 0.0f });
				}
			}
		}

		m_lifeTimer += fixedDt;
		if (m_duration > 0.0f && m_lifeTimer >= m_duration)
		{
			PoolManager::GetInstance()->Despawn<GameObject>(m_poolKey, &gameObject);
		}
	}
	else
	{
		float stepLength = m_speed * fixedDt;
		m_traveledDistance += stepLength;

		if (m_traveledDistance >= m_range)
		{
			TriggerOnHitExplosion(transform.GetPosition());
			PoolManager::GetInstance()->Despawn<GameObject>(m_poolKey, &gameObject);
		}
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
		if (m_hitCooldowns.find(instanceID) != m_hitCooldowns.end())
		{
			return;
		}

		m_hitCooldowns[instanceID] = 0.4f; // 0.4초 쿨다운
		pDamageable->TakeDamage(m_damage, m_pAttacker.Get());

		if (!m_effectKey.empty())
		{
			GameObject* pVFX = PoolManager::GetInstance()->Spawn<GameObject>(m_effectKey);
			if (pVFX)
			{
				pVFX->transform.SetPosition(transform.GetPosition().x, transform.GetPosition().y);
			}
		}

		if (!m_onHitAnimKey.empty())
		{
			TriggerOnHitExplosion(transform.GetPosition());
		}

		if (!m_isOrbital && m_penetrationCount > 0)
		{
			m_penetrationCount--;
			if (m_penetrationCount <= 0)
			{
				PoolManager::GetInstance()->Despawn<GameObject>(m_poolKey, &gameObject);
			}
		}
	}
}

void ProjectileComponent::TriggerOnHitExplosion(const Vector2& pos)
{
	if (m_onHitAnimKey.empty()) return;

	GameObject* pAoEObj = PoolManager::GetInstance()->Spawn<GameObject>("GenericAoEPrefab");
	if (pAoEObj)
	{
		pAoEObj->transform.SetPosition(pos.x, pos.y);
		AoEComponent* pAoE = pAoEObj->GetComponent<AoEComponent>();
		if (!pAoE)
		{
			pAoE = pAoEObj->AddComponent<AoEComponent>();
		}
		if (pAoE)
		{
			SkillLevelData aoeData;
			aoeData.damage = m_damage * m_onHitDamageRatio;
			aoeData.range = m_onHitRadius;
			aoeData.duration = m_onHitDuration;
			aoeData.cooldown = 0.0f;
			pAoE->InitWithAnim(m_onHitAnimKey, aoeData, m_pAttacker.Get(), "GenericAoEPrefab");
		}
	}
}