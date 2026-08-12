#include "Engine/Core/pch.h"
#include "SkillComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Manager/DataManager.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Manager/PrefabManager.h"
#include "Engine/Physics/PhysicsManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/ColliderComponent.h"
#include "Engine/Framework/Components/Physics/CircleCollider.h"
#include "Game/Monster/Monster.h"
#include "Game/Manager/InGameManager.h"
#include "Game/Skill/ProjectileComponent.h"
#include "Game/Skill/AuraComponent.h"
#include "Game/Skill/AoEComponent.h"

static ComponentRegistrar<SkillComponent> registrar(EngineKey::CustomComponent::SkillComponent.data());

SkillComponent::SkillComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
}

void SkillComponent::Start()
{
	if (m_skills.empty())
	{
		AddSkill(m_defaultSkillID);
	}
}

void SkillComponent::AddSkill(uint32 skillAssetID)
{
	auto pSkillSO = DataManager::GetInstance()->GetSkillSO(skillAssetID);
	if (pSkillSO)
	{
		AddSkill(pSkillSO);
	}
}

void SkillComponent::AddSkill(std::shared_ptr<const SkillSO> pSkillSO)
{
	if (!pSkillSO) return;

	for (auto& inst : m_skills)
	{
		if (inst.pSO && inst.pSO->GetSkillID() == pSkillSO->GetSkillID())
		{
			inst.level++;
			return;
		}
	}

	SkillInstance newInst;
	newInst.pSO = pSkillSO;
	newInst.level = 1;
	newInst.cooldownTimer = 0.0f;
	m_skills.push_back(newInst);
}

void SkillComponent::UpgradeSkill(uint32 skillAssetID)
{
	for (auto& inst : m_skills)
	{
		if (inst.pSO && inst.pSO->GetSkillID() == skillAssetID)
		{
			inst.level++;
			return;
		}
	}
}

bool SkillComponent::HasSkill(uint32 skillAssetID) const
{
	for (const auto& inst : m_skills)
	{
		if (inst.pSO && inst.pSO->GetSkillID() == skillAssetID)
			return true;
	}
	return false;
}

int32 SkillComponent::GetSkillLevel(uint32 skillAssetID) const
{
	for (const auto& inst : m_skills)
	{
		if (inst.pSO && inst.pSO->GetSkillID() == skillAssetID)
			return inst.level;
	}
	return 0;
}

void SkillComponent::FixedUpdate(float fixedDt)
{
	if (!gameObject.IsActive() || !IsEnabled()) return;

	// 게임 진행 상태 확인 (InGameManager가 활성 중이고 게임 시작 전/카운트다운 중이면 스킬 발동 중지)
	if (InGameManager* pInGameMgr = InGameManager::GetInstance())
	{
		if (!pInGameMgr->IsGameStarted())
		{
			return;
		}
	}

	for (auto& inst : m_skills)
	{
		if (!inst.pSO) continue;

		const auto& levelData = inst.GetCurrentLevelData();
		inst.cooldownTimer += fixedDt;

		if (inst.cooldownTimer >= levelData.cooldown)
		{
			inst.cooldownTimer = 0.0f;
			CastSkill(inst);
		}
	}
}

GameObject* SkillComponent::FindClosestMonster(float maxRange) const
{
	Vector2 myPos = transform.GetPosition();
	auto colliders = PhysicsManager::GetInstance()->OverlapAABB(myPos, maxRange, PhysicsLayer::Monster);

	GameObject* pClosestObj = nullptr;
	float minDistSq = maxRange * maxRange;

	for (ColliderComponent* pCol : colliders)
	{
		if (!pCol || !pCol->IsEnabled() || !pCol->gameObject.IsActive()) continue;

		Monster* pMonster = pCol->gameObject.GetComponent<Monster>();
		if (pMonster && !pMonster->IsDead())
		{
			Vector2 mPos = pMonster->transform.GetPosition();
			float distSq = (mPos - myPos).LengthSquared();
			if (distSq < minDistSq)
			{
				minDistSq = distSq;
				pClosestObj = &pCol->gameObject;
			}
		}
	}
	return pClosestObj;
}

void SkillComponent::CastSkill(SkillInstance& instance)
{
	if (!instance.pSO) return;
	const auto& levelData = instance.GetCurrentLevelData();

	switch (instance.pSO->GetCategory())
	{
	case ESkillCategory::Projectile:
	{
		GameObject* pTargetMonster = FindClosestMonster(levelData.range * 1.2f);
		CastProjectileSkill(instance, levelData, pTargetMonster);
		break;
	}
	case ESkillCategory::Aura:
		CastAuraSkill(instance, levelData);
		break;
	case ESkillCategory::GroundArea:
	{
		GameObject* pTargetMonster = FindClosestMonster(levelData.range * 1.2f);
		CastGroundAreaSkill(instance, levelData, pTargetMonster);
		break;
	}
	}
}

void SkillComponent::CastProjectileSkill(const SkillInstance& instance, const SkillLevelData& data, GameObject* pTargetMonster)
{
	Vector2 myPos = transform.GetPosition();
	Vector2 baseDir = { 1.0f, 0.0f };

	if (pTargetMonster != nullptr)
	{
		Vector2 targetPos = pTargetMonster->transform.GetPosition();
		baseDir = (targetPos - myPos).GetNormalized();
	}

	int32 count = (std::max)(1, data.projectileCount);
	float totalSpreadAngle = (count > 1) ? 30.0f : 0.0f;
	float startAngle = -totalSpreadAngle / 2.0f;
	float angleStep = (count > 1) ? (totalSpreadAngle / (count - 1)) : 0.0f;

	float baseRad = atan2f(baseDir.y, baseDir.x);
	std::string poolKey = instance.pSO->GetPrefabKey();
	if (poolKey.empty() || poolKey == "DefaultProjectile" || !PoolManager::GetInstance()->HasPool(poolKey))
	{
		poolKey = "GenericProjectilePrefab";
	}

	for (int32 i = 0; i < count; ++i)
	{
		float offsetDeg = startAngle + i * angleStep;
		float finalRad = baseRad + DegreeToRadian(offsetDeg);
		Vector2 fireDir = { cosf(finalRad), sinf(finalRad) };

		GameObject* pProjObj = PoolManager::GetInstance()->Spawn<GameObject>(poolKey);
		if (!pProjObj)
		{
			std::cout << "[SkillComponent] Error: Failed to spawn projectile prefab from pool: " << poolKey << std::endl;
			continue;
		}

		pProjObj->transform.SetPosition(myPos.x, myPos.y);

		ProjectileComponent* pProj = pProjObj->GetComponent<ProjectileComponent>();
		if (!pProj)
		{
			std::cout << "[SkillComponent] Error: Prefab missing ProjectileComponent: " << poolKey << std::endl;
			continue;
		}

		pProj->Init(fireDir, data, instance.pSO.get(), &gameObject, poolKey);
	}
}

void SkillComponent::CastAuraSkill(const SkillInstance& instance, const SkillLevelData& data)
{
	std::string poolKey = instance.pSO->GetPrefabKey();
	if (poolKey.empty() || poolKey == "DefaultProjectile" || !PoolManager::GetInstance()->HasPool(poolKey))
	{
		poolKey = "GenericAuraPrefab";
	}

	GameObject* pAuraObj = PoolManager::GetInstance()->Spawn<GameObject>(poolKey);
	if (!pAuraObj)
	{
		std::cout << "[SkillComponent] Error: Failed to spawn aura prefab from pool: " << poolKey << std::endl;
		return;
	}

	pAuraObj->transform.SetPosition(transform.GetPosition().x, transform.GetPosition().y);

	AuraComponent* pAura = pAuraObj->GetComponent<AuraComponent>();
	if (!pAura)
	{
		std::cout << "[SkillComponent] Error: Prefab missing AuraComponent: " << poolKey << std::endl;
		return;
	}

	pAura->Init(data, instance.pSO.get(), &gameObject, poolKey);
}

void SkillComponent::CastGroundAreaSkill(const SkillInstance& instance, const SkillLevelData& data, GameObject* pTargetMonster)
{
	Vector2 spawnPos = transform.GetPosition();
	if (pTargetMonster != nullptr)
	{
		spawnPos = pTargetMonster->transform.GetPosition();
	}

	std::string poolKey = instance.pSO->GetPrefabKey();
	if (poolKey.empty() || poolKey == "DefaultProjectile" || !PoolManager::GetInstance()->HasPool(poolKey))
	{
		poolKey = "GenericAoEPrefab";
	}

	GameObject* pAoEObj = PoolManager::GetInstance()->Spawn<GameObject>(poolKey);
	if (!pAoEObj)
	{
		std::cout << "[SkillComponent] Error: Failed to spawn AoE prefab from pool: " << poolKey << std::endl;
		return;
	}

	AoEComponent* pAoE = pAoEObj->GetComponent<AoEComponent>();
	if (!pAoE)
	{
		std::cout << "[SkillComponent] Error: Prefab missing AoEComponent: " << poolKey << std::endl;
		return;
	}

	pAoEObj->transform.SetPosition(spawnPos);
	pAoE->Init(data, instance.pSO.get(), &gameObject, poolKey);
}
