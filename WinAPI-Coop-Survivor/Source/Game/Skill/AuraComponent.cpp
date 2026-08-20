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
#include "Engine/Framework/Components/Physics/BoxCollider.h"
#include "Engine/Framework/Components/Physics/CircleCollider.h"
#include "Engine/Framework/Components/Render/AnimatorComponent.h"

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
	m_tickInterval = 0.3f;
	m_tickTimer = m_tickInterval;
	m_lifeTimer = 0.0f;
	m_pCaster = pCaster;
	m_poolKey = poolKey;
	m_rotSpeed = (data.speed > 0.0f) ? data.speed : 360.0f; // 360 deg/sec for fast, energetic rotation

	// Visual Scaling based on range (Rings base sprite frame radius ~16px)
	float baseRadius = 16.0f;
	float scaleFactor = (m_range > 0.0f) ? (m_range / baseRadius) : 1.0f;
	transform.SetScale({ scaleFactor, scaleFactor });

	if (CircleCollider* pCol = gameObject.GetComponent<CircleCollider>())
	{
		pCol->SetRadius(m_range);
		if (b2Body_IsValid(pCol->GetBodyId()))
		{
			Vector2 pos = transform.GetPosition();
			b2Body_SetTransform(pCol->GetBodyId(), { PixelToMeter(pos.x), PixelToMeter(pos.y) }, b2Rot_identity);
		}
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

void AuraComponent::FixedUpdate(float fixedDt)
{
	if (!gameObject.IsActive()) return;

	float newRotDeg = transform.GetLocalRotation() + m_rotSpeed * fixedDt;
	transform.SetRotation(newRotDeg);

	if (m_pCaster.IsValid())
	{
		Vector2 pos = m_pCaster->transform.GetPosition();
		transform.SetPosition(pos.x, pos.y);

		if (ColliderComponent* pCol = gameObject.GetComponent<ColliderComponent>())
		{
			if (b2Body_IsValid(pCol->GetBodyId()))
			{
				b2Rot b2Rot = b2MakeRot(DegreeToRadian(newRotDeg));
				b2Body_SetTransform(pCol->GetBodyId(), { PixelToMeter(pos.x), PixelToMeter(pos.y) }, b2Rot);
			}
		}
	}

	m_tickTimer += fixedDt;
	if (m_tickTimer >= m_tickInterval)
	{
		m_tickTimer -= m_tickInterval;
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
	auto colliders = PhysicsManager::GetInstance()->OverlapAABB(myPos, m_range + 30.0f, PhysicsLayer::Monster);

	for (ColliderComponent* pCol : colliders)
	{
		if (!pCol || !pCol->IsEnabled() || !pCol->gameObject.IsActive()) continue;

		IDamageable* pDamageable = pCol->gameObject.GetComponent<IDamageable>();
		if (pDamageable && !pDamageable->IsDead())
		{
			Vector2 monsterPos = pCol->transform.GetPosition();
			float distSq = (monsterPos - myPos).LengthSquared();

			float monsterRadius = 15.0f;
			if (auto* circle = pCol->gameObject.GetComponent<CircleCollider>())
			{
				monsterRadius = circle->GetRadius();
			}
			else if (auto* box = pCol->gameObject.GetComponent<BoxCollider>())
			{
				Vector2 sz = box->GetSize();
				monsterRadius = (std::max)(sz.x, sz.y) * 0.5f;
			}

			float totalRange = m_range + monsterRadius;
			if (distSq <= (totalRange * totalRange))
			{
				pDamageable->TakeDamage(m_damage, m_pCaster.Get());
			}
		}
	}
}
