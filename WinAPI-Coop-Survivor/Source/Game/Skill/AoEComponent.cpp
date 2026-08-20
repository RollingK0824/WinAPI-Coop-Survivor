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

#include "Engine/Framework/Components/Physics/BoxCollider.h"
#include "Engine/Framework/Components/Physics/CircleCollider.h"
#include "Engine/Framework/Components/Render/AnimatorComponent.h"

static ComponentRegistrar<AoEComponent> registrar(EngineKey::CustomComponent::AoEComponent.data());

AoEComponent::AoEComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
}

void AoEComponent::Init(const SkillLevelData& data, const SkillSO* pSO, GameObject* attacker, const std::string& poolKey, bool flipX)
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

	// Visual Scaling based on range (VFX sprite frame base radius ~35px)
	float baseRadius = 35.0f;
	float scaleFactor = (m_range > 0.0f) ? (m_range / baseRadius) : 1.0f;
	transform.SetScale({ scaleFactor, scaleFactor });

	if (BoxCollider* pBox = gameObject.GetComponent<BoxCollider>())
	{
		float boxW = m_range * 2.0f;
		float boxH = m_range * 0.7f;
		pBox->SetSize(boxW, boxH);
		if (b2Body_IsValid(pBox->GetBodyId()))
		{
			Vector2 pos = transform.GetPosition();
			b2Body_SetTransform(pBox->GetBodyId(), { PixelToMeter(pos.x), PixelToMeter(pos.y) }, b2Rot_identity);
		}
	}
	else if (CircleCollider* pCol = gameObject.GetComponent<CircleCollider>())
	{
		pCol->SetRadius(m_range);
		if (b2Body_IsValid(pCol->GetBodyId()))
		{
			Vector2 pos = transform.GetPosition();
			b2Body_SetTransform(pCol->GetBodyId(), { PixelToMeter(pos.x), PixelToMeter(pos.y) }, b2Rot_identity);
		}
	}

	if (auto renderer = gameObject.GetComponent<SpriteRendererComponent>())
	{
		renderer->SetSize({ 0.0f, 0.0f });
		renderer->SetFlip(flipX, false);
	}

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
}

void AoEComponent::InitWithAnim(const std::string& animKey, const SkillLevelData& data, GameObject* attacker, const std::string& poolKey, bool flipX)
{
	m_damage = data.damage;
	m_range = data.range;
	m_duration = (data.duration > 0.0f) ? data.duration : 0.4f;
	m_tickInterval = 0.0f;
	m_tickTimer = 0.0f;
	m_lifeTimer = 0.0f;
	m_hasAppliedDamage = false;
	m_pAttacker = attacker;
	m_poolKey = poolKey;

	float baseRadius = 35.0f;
	float scaleFactor = (m_range > 0.0f) ? (m_range / baseRadius) : 1.0f;
	transform.SetScale({ scaleFactor, scaleFactor });

	if (BoxCollider* pBox = gameObject.GetComponent<BoxCollider>())
	{
		float boxW = m_range * 2.0f;
		float boxH = m_range * 0.7f;
		pBox->SetSize(boxW, boxH);
		if (b2Body_IsValid(pBox->GetBodyId()))
		{
			Vector2 pos = transform.GetPosition();
			b2Body_SetTransform(pBox->GetBodyId(), { PixelToMeter(pos.x), PixelToMeter(pos.y) }, b2Rot_identity);
		}
	}
	else if (CircleCollider* pCol = gameObject.GetComponent<CircleCollider>())
	{
		pCol->SetRadius(m_range);
		if (b2Body_IsValid(pCol->GetBodyId()))
		{
			Vector2 pos = transform.GetPosition();
			b2Body_SetTransform(pCol->GetBodyId(), { PixelToMeter(pos.x), PixelToMeter(pos.y) }, b2Rot_identity);
		}
	}

	if (auto renderer = gameObject.GetComponent<SpriteRendererComponent>())
	{
		renderer->SetSize({ 0.0f, 0.0f });
		renderer->SetFlip(flipX, false);
	}

	AnimatorComponent* pAnim = gameObject.GetComponent<AnimatorComponent>();
	if (!pAnim)
	{
		pAnim = gameObject.AddComponent<AnimatorComponent>();
	}
	if (pAnim)
	{
		std::wstring wKey(animKey.begin(), animKey.end());
		pAnim->Play(wKey, true);
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

	if (BoxCollider* pBox = gameObject.GetComponent<BoxCollider>())
	{
		Vector2 boxSize = pBox->GetSize();
		float halfW = boxSize.x * 0.5f;
		float halfH = boxSize.y * 0.5f;
		float maxExtent = (std::max)(halfW, halfH) + 30.0f;
		auto colliders = PhysicsManager::GetInstance()->OverlapAABB(centerPos, maxExtent, PhysicsLayer::Monster);

		for (ColliderComponent* pCol : colliders)
		{
			if (!pCol || !pCol->IsEnabled() || !pCol->gameObject.IsActive()) continue;

			IDamageable* pDamageable = pCol->gameObject.GetComponent<IDamageable>();
			if (pDamageable && !pDamageable->IsDead())
			{
				Vector2 targetPos = pCol->transform.GetPosition();
				float monsterExtX = 12.0f;
				float monsterExtY = 12.0f;
				if (auto* monsterBox = pCol->gameObject.GetComponent<BoxCollider>())
				{
					Vector2 sz = monsterBox->GetSize();
					monsterExtX = sz.x * 0.5f;
					monsterExtY = sz.y * 0.5f;
				}
				else if (auto* monsterCircle = pCol->gameObject.GetComponent<CircleCollider>())
				{
					float r = monsterCircle->GetRadius();
					monsterExtX = r;
					monsterExtY = r;
				}

				if (std::abs(targetPos.x - centerPos.x) <= (halfW + monsterExtX) &&
					std::abs(targetPos.y - centerPos.y) <= (halfH + monsterExtY))
				{
					pDamageable->TakeDamage(m_damage, m_pAttacker.Get());
				}
			}
		}
	}
	else
	{
		auto colliders = PhysicsManager::GetInstance()->OverlapAABB(centerPos, m_range + 30.0f, PhysicsLayer::Monster);

		for (ColliderComponent* pCol : colliders)
		{
			if (!pCol || !pCol->IsEnabled() || !pCol->gameObject.IsActive()) continue;

			IDamageable* pDamageable = pCol->gameObject.GetComponent<IDamageable>();
			if (pDamageable && !pDamageable->IsDead())
			{
				Vector2 targetPos = pCol->transform.GetPosition();
				float distSq = (targetPos - centerPos).LengthSquared();

				float monsterRadius = 15.0f;
				if (auto* monsterCircle = pCol->gameObject.GetComponent<CircleCollider>())
				{
					monsterRadius = monsterCircle->GetRadius();
				}
				else if (auto* monsterBox = pCol->gameObject.GetComponent<BoxCollider>())
				{
					Vector2 sz = monsterBox->GetSize();
					monsterRadius = (std::max)(sz.x, sz.y) * 0.5f;
				}

				float totalRange = m_range + monsterRadius;
				if (distSq <= (totalRange * totalRange))
				{
					pDamageable->TakeDamage(m_damage, m_pAttacker.Get());
				}
			}
		}
	}
}
