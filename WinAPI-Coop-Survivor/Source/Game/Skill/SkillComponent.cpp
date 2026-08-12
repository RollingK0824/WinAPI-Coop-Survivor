#include "Engine/Core/pch.h"
#include "SkillComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Manager/DataManager.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/CircleCollider.h"
#include "Game/Monster/Monster.h"
#include "Game/Skill/ProjectileComponent.h"
#include "Game/Skill/AuraComponent.h"
#include "Game/Skill/AoEComponent.h"

static ComponentRegistrar<SkillComponent> registrar(EngineKey::CustomComponent::SkillComponent.data());

SkillComponent::SkillComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeVariable("DefaultSkillID", &m_defaultSkillID);
}

void SkillComponent::Start()
{
	if (m_defaultSkillID != 0 && !HasSkill(m_defaultSkillID))
	{
		AddSkill(m_defaultSkillID);
	}
}

void SkillComponent::AddSkill(uint32 skillAssetID)
{
	auto pSO = DataManager::GetInstance()->GetSkillSO(skillAssetID);
	if (pSO)
	{
		AddSkill(pSO);
	}
}

void SkillComponent::AddSkill(std::shared_ptr<const SkillSO> pSO)
{
	if (!pSO) return;

	for (auto& inst : m_skills)
	{
		if (inst.pSO && inst.pSO->GetSkillID() == pSO->GetSkillID())
		{
			inst.level++;
			return;
		}
	}

	SkillInstance newInst;
	newInst.pSO = pSO;
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
			if (inst.level < inst.pSO->GetMaxLevel())
			{
				inst.level++;
			}
			return;
		}
	}

	AddSkill(skillAssetID);
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
	if (!gameObject.IsActive()) return;

	for (auto& inst : m_skills)
	{
		if (!inst.pSO) continue;

		const auto& levelData = inst.GetCurrentLevelData();
		inst.cooldownTimer += fixedDt;

		if (inst.cooldownTimer >= levelData.cooldown)
		{
			inst.cooldownTimer = 0.0f;
			GameObject* pClosestMonster = FindClosestMonster(levelData.range * 1.2f);
			CastSkill(inst, pClosestMonster);
		}
	}
}

GameObject* SkillComponent::FindClosestMonster(float maxRange) const
{
	Scene* pScene = SceneManager::GetInstance()->GetActiveScene();
	if (!pScene) return nullptr;

	Vector2 myPos = transform.GetPosition();
	GameObject* pClosestObj = nullptr;
	float minDistSq = maxRange * maxRange;

	const auto& objects = pScene->GetGameObjects();
	for (GameObject* pObj : objects)
	{
		if (!pObj || !pObj->IsActive() || pObj->IsDead()) continue;

		Monster* pMonster = pObj->GetComponent<Monster>();
		if (pMonster && !pMonster->IsDead())
		{
			Vector2 mPos = pObj->transform.GetPosition();
			float distSq = (mPos - myPos).LengthSquared();
			if (distSq < minDistSq)
			{
				minDistSq = distSq;
				pClosestObj = pObj;
			}
		}
	}
	return pClosestObj;
}

void SkillComponent::CastSkill(SkillInstance& instance, GameObject* pTargetMonster)
{
	if (!instance.pSO) return;
	const auto& levelData = instance.GetCurrentLevelData();

	switch (instance.pSO->GetCategory())
	{
	case ESkillCategory::Projectile:
		CastProjectileSkill(instance, levelData, pTargetMonster);
		break;
	case ESkillCategory::Aura:
		CastAuraSkill(instance, levelData);
		break;
	case ESkillCategory::GroundArea:
		CastGroundAreaSkill(instance, levelData, pTargetMonster);
		break;
	}
}

void SkillComponent::CastProjectileSkill(const SkillInstance& instance, const SkillLevelData& data, GameObject* pTargetMonster)
{
	Scene* pScene = SceneManager::GetInstance()->GetActiveScene();
	if (!pScene) return;

	Vector2 myPos = transform.GetPosition();
	Vector2 baseDir = { 1.0f, 0.0f };

	if (pTargetMonster != nullptr)
	{
		Vector2 targetPos = pTargetMonster->transform.GetPosition();
		baseDir = (targetPos - myPos).GetNormalized();
	}

	int32 count = (std::max)(1, data.projectileCount);
	float totalSpreadAngle = (count > 1) ? 30.0f : 0.0f; // 부채꼴 분산 각도
	float startAngle = -totalSpreadAngle / 2.0f;
	float angleStep = (count > 1) ? (totalSpreadAngle / (count - 1)) : 0.0f;

	float baseRad = atan2f(baseDir.y, baseDir.x);

	for (int32 i = 0; i < count; ++i)
	{
		float offsetDeg = startAngle + i * angleStep;
		float finalRad = baseRad + (offsetDeg * 3.14159265f / 180.0f);
		Vector2 fireDir = { cosf(finalRad), sinf(finalRad) };

		GameObject* pProjObj = pScene->CreateGameObject("SkillProjectile");
		pProjObj->transform.SetPosition(myPos.x, myPos.y);

		CircleCollider* pCol = pProjObj->AddComponent<CircleCollider>();
		pCol->SetOffset({ 0.0f, 0.0f });
		pCol->m_bIsSensor = true;
		pCol->SetFilter(PhysicsLayer::Projectile, PhysicsLayer::Monster);

		ProjectileComponent* pProj = pProjObj->AddComponent<ProjectileComponent>();
		pProj->Init(fireDir, data.speed, data.damage, data.penetrationCount, data.range, &gameObject, "ProjectilePool");
	}
}

void SkillComponent::CastAuraSkill(const SkillInstance& instance, const SkillLevelData& data)
{
	Scene* pScene = SceneManager::GetInstance()->GetActiveScene();
	if (!pScene) return;

	GameObject* pAuraObj = pScene->CreateGameObject("SkillAura");
	pAuraObj->transform.SetPosition(transform.GetPosition().x, transform.GetPosition().y);

	AuraComponent* pAura = pAuraObj->AddComponent<AuraComponent>();
	pAura->Init(data.damage, data.range, data.duration, &gameObject);
}

void SkillComponent::CastGroundAreaSkill(const SkillInstance& instance, const SkillLevelData& data, GameObject* pTargetMonster)
{
	Scene* pScene = SceneManager::GetInstance()->GetActiveScene();
	if (!pScene) return;

	Vector2 spawnPos = transform.GetPosition();
	if (pTargetMonster != nullptr)
	{
		spawnPos = pTargetMonster->transform.GetPosition();
	}

	GameObject* pAoEObj = pScene->CreateGameObject("SkillAoE");

	AoEComponent* pAoE = pAoEObj->AddComponent<AoEComponent>();
	pAoE->Init(data.damage, data.range, data.duration, spawnPos, &gameObject);
}
