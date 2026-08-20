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
#include "Game/Player/Player.h"
#include "Game/Manager/InGameManager.h"
#include "Game/Skill/ProjectileComponent.h"
#include "Game/Skill/AuraComponent.h"
#include "Game/Skill/AoEComponent.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"

#include "Engine/Core/EventBus.h"
#include "Game/Manager/GameEvents.h"

static ComponentRegistrar<SkillComponent> registrar(EngineKey::CustomComponent::SkillComponent.data());

SkillComponent::SkillComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	m_skills.reserve(MAX_SKILL_SLOTS);
}

void SkillComponent::Start()
{
	m_pPlayer = gameObject.GetComponent<Player>();

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

	for (size_t i = 0; i < m_skills.size(); ++i)
	{
		if (m_skills[i].pSO && m_skills[i].pSO->GetSkillID() == pSkillSO->GetSkillID())
		{
			m_skills[i].level++;

			OnSkillSlotChangedEvent evt;
			evt.playerNetID = 0;
			evt.slotIndex = static_cast<uint8>(i);
			evt.skillID = m_skills[i].pSO->GetSkillID();
			evt.level = static_cast<uint8>(m_skills[i].level);
			EventBus::GetInstance()->Publish(evt);
			return;
		}
	}

	if (m_skills.size() >= MAX_SKILL_SLOTS) return; 

	SkillInstance newInst;
	newInst.pSO = pSkillSO;
	newInst.level = 1;
	newInst.cooldownTimer = 0.0f;
	m_skills.push_back(newInst);

	size_t slotIdx = m_skills.size() - 1;

	OnSkillSlotChangedEvent evt;
	evt.playerNetID = 0;
	evt.slotIndex = static_cast<uint8>(slotIdx);
	evt.skillID = pSkillSO->GetSkillID();
	evt.level = 1;
	EventBus::GetInstance()->Publish(evt);
}

void SkillComponent::UpgradeSkill(uint32 skillAssetID)
{
	for (size_t i = 0; i < m_skills.size(); ++i)
	{
		if (m_skills[i].pSO && m_skills[i].pSO->GetSkillID() == skillAssetID)
		{
			m_skills[i].level++;

			OnSkillSlotChangedEvent evt;
			evt.playerNetID = 0;
			evt.slotIndex = static_cast<uint8>(i);
			evt.skillID = m_skills[i].pSO->GetSkillID();
			evt.level = static_cast<uint8>(m_skills[i].level);
			EventBus::GetInstance()->Publish(evt);
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
	if (m_pPlayer.IsValid() && m_pPlayer->IsDead()) return;

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
		if (pMonster && !pMonster->IsDead() && pMonster->gameObject.IsActive())
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

	if (!pClosestObj)
	{
		Scene* pScene = gameObject.GetOwnerScene();
		if (pScene)
		{
			for (auto* obj : pScene->GetGameObjects())
			{
				if (!obj || !obj->IsActive() || obj->IsDead()) continue;

				Monster* pMonster = obj->GetComponent<Monster>();
				if (pMonster && !pMonster->IsDead() && pMonster->gameObject.IsActive())
				{
					Vector2 mPos = pMonster->transform.GetPosition();
					float distSq = (mPos - myPos).LengthSquared();
					if (distSq < minDistSq)
					{
						minDistSq = distSq;
						pClosestObj = obj;
					}
				}
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
	Vector2 facingDir = { 1.0f, 0.0f };
	if (m_pPlayer.IsValid())
	{
		facingDir = m_pPlayer->GetFacingDirection();
	}

	Vector2 baseDir = facingDir;
	if (data.aimType == EAimType::NearestEnemy)
	{
		if (pTargetMonster != nullptr)
		{
			Vector2 targetPos = pTargetMonster->transform.GetPosition();
			baseDir = (targetPos - myPos).GetNormalized();
		}
	}
	else if (data.aimType == EAimType::FixedAngle)
	{
		float rad = DegreeToRadian(data.fixedAngleDeg);
		baseDir = { cosf(rad), sinf(rad) };
	}

	int32 count = (std::max)(1, data.projectileCount);
	float spreadAngle = data.spreadAngle;

	std::string poolKey = instance.pSO->GetPrefabKey();
	if (poolKey.empty() || poolKey == "DefaultProjectile" || !PoolManager::GetInstance()->HasPool(poolKey))
	{
		poolKey = "GenericProjectilePrefab";
	}

	if (data.aimType == EAimType::Orbital)
	{
		float angleStep = (2.0f * 3.14159265f) / count;
		for (int32 i = 0; i < count; ++i)
		{
			float angle = i * angleStep;
			Vector2 initDir = { cosf(angle), sinf(angle) };

			GameObject* pProjObj = PoolManager::GetInstance()->Spawn<GameObject>(poolKey);
			if (!pProjObj) continue;

			pProjObj->transform.SetPosition(myPos.x, myPos.y);

			ProjectileComponent* pProj = pProjObj->GetComponent<ProjectileComponent>();
			if (pProj)
			{
				pProj->Init(initDir, data, instance.pSO.get(), &gameObject, poolKey);
			}
		}
		return;
	}

	if (count == 2 && spreadAngle >= 179.0f)
	{
		Vector2 dirs[2] = { baseDir, -baseDir };
		for (int32 i = 0; i < 2; ++i)
		{
			GameObject* pProjObj = PoolManager::GetInstance()->Spawn<GameObject>(poolKey);
			if (!pProjObj) continue;
			pProjObj->transform.SetPosition(myPos.x, myPos.y);

			ProjectileComponent* pProj = pProjObj->GetComponent<ProjectileComponent>();
			if (pProj)
			{
				pProj->Init(dirs[i], data, instance.pSO.get(), &gameObject, poolKey);
			}
		}
		return;
	}

	float baseRad = atan2f(baseDir.y, baseDir.x);
	float totalSpreadRad = DegreeToRadian(spreadAngle);
	float startAngle = (count > 1) ? (-totalSpreadRad / 2.0f) : 0.0f;
	float angleStep = (count > 1) ? (totalSpreadRad / (count - 1)) : 0.0f;

	for (int32 i = 0; i < count; ++i)
	{
		float offsetRad = startAngle + i * angleStep;
		float finalRad = baseRad + offsetRad;
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

	if (ColliderComponent* pCol = pAuraObj->GetComponent<ColliderComponent>())
	{
		if (b2Body_IsValid(pCol->GetBodyId()))
		{
			Vector2 myPos = transform.GetPosition();
			b2Body_SetTransform(pCol->GetBodyId(), { PixelToMeter(myPos.x), PixelToMeter(myPos.y) }, b2Rot_identity);
		}
	}

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
	Vector2 myPos = transform.GetPosition();
	Vector2 facingDir = { 1.0f, 0.0f };
	if (m_pPlayer.IsValid())
	{
		facingDir = m_pPlayer->GetFacingDirection();
	}

	std::string poolKey = instance.pSO->GetPrefabKey();
	if (poolKey.empty() || poolKey == "DefaultProjectile" || !PoolManager::GetInstance()->HasPool(poolKey))
	{
		poolKey = "GenericAoEPrefab";
	}

	float offsetDistance = (data.range > 0.0f) ? (data.range * 0.5f) : 60.0f;
	int32 count = (std::max)(1, data.projectileCount);
	float spreadAngle = data.spreadAngle;

	struct AoESpawnConfig
	{
		Vector2 position;
		float rotation = 0.0f;
		bool flipX = false;
	};

	std::vector<AoESpawnConfig> spawnConfigs;

	bool isPetalSlash = (instance.pSO && (instance.pSO->GetSkillID() == 303 || instance.pSO->GetAssetName() == "PetalSlash"));

	if (isPetalSlash)
	{
		bool isFacingLeft = (facingDir.x < -0.01f);
		if (std::abs(facingDir.x) <= 0.01f && m_pPlayer.IsValid())
		{
			if (auto sprite = m_pPlayer->gameObject.GetComponent<SpriteRendererComponent>())
			{
				isFacingLeft = sprite->GetFlipX();
			}
		}

		if (count == 1)
		{
			float sign = isFacingLeft ? -1.0f : 1.0f;
			spawnConfigs.push_back({ myPos + Vector2(sign * offsetDistance, 0.0f), 0.0f, isFacingLeft });
		}
		else if (count == 2)
		{
			float sign1 = isFacingLeft ? -1.0f : 1.0f;
			float sign2 = isFacingLeft ? 1.0f : -1.0f;
			spawnConfigs.push_back({ myPos + Vector2(sign1 * offsetDistance, 0.0f), 0.0f, isFacingLeft });
			spawnConfigs.push_back({ myPos + Vector2(sign2 * offsetDistance, 0.0f), 0.0f, !isFacingLeft });
		}
		else
		{
			for (int32 i = 0; i < count; ++i)
			{
				bool flip = (i % 2 == 0) ? isFacingLeft : !isFacingLeft;
				float sign = flip ? -1.0f : 1.0f;
				float yOffset = (i >= 2) ? ((i % 2 == 0 ? -1.0f : 1.0f) * 15.0f * (i / 2)) : 0.0f;
				spawnConfigs.push_back({ myPos + Vector2(sign * offsetDistance, yOffset), 0.0f, flip });
			}
		}
	}
	else if (data.aimType == EAimType::NearestEnemy)
	{
		if (pTargetMonster != nullptr)
		{
			spawnConfigs.push_back({ pTargetMonster->transform.GetPosition(), 0.0f, false });
		}
		else
		{
			float rot = atan2f(facingDir.y, facingDir.x);
			spawnConfigs.push_back({ myPos + facingDir * offsetDistance, rot, false });
		}
	}
	else if (data.aimType == EAimType::OwnerFacing)
	{
		float baseRad = atan2f(facingDir.y, facingDir.x);
		if (count == 2 && spreadAngle >= 179.0f)
		{
			spawnConfigs.push_back({ myPos + facingDir * offsetDistance, baseRad, false });
			spawnConfigs.push_back({ myPos - facingDir * offsetDistance, baseRad + 3.14159265f, false });
		}
		else
		{
			float totalSpreadRad = DegreeToRadian(spreadAngle);
			float startAngle = (count > 1) ? (-totalSpreadRad / 2.0f) : 0.0f;
			float angleStep = (count > 1) ? (totalSpreadRad / (count - 1)) : 0.0f;

			for (int32 i = 0; i < count; ++i)
			{
				float finalRad = baseRad + startAngle + i * angleStep;
				Vector2 dir = { cosf(finalRad), sinf(finalRad) };
				spawnConfigs.push_back({ myPos + dir * offsetDistance, finalRad, false });
			}
		}
	}
	else if (data.aimType == EAimType::FixedAngle)
	{
		float baseRad = DegreeToRadian(data.fixedAngleDeg);
		Vector2 fixedDir = { cosf(baseRad), sinf(baseRad) };

		if (count == 2 && spreadAngle >= 179.0f)
		{
			spawnConfigs.push_back({ myPos + fixedDir * offsetDistance, baseRad, false });
			spawnConfigs.push_back({ myPos - fixedDir * offsetDistance, baseRad + 3.14159265f, false });
		}
		else
		{
			float totalSpreadRad = DegreeToRadian(spreadAngle);
			float startAngle = (count > 1) ? (-totalSpreadRad / 2.0f) : 0.0f;
			float angleStep = (count > 1) ? (totalSpreadRad / (count - 1)) : 0.0f;

			for (int32 i = 0; i < count; ++i)
			{
				float finalRad = baseRad + startAngle + i * angleStep;
				Vector2 dir = { cosf(finalRad), sinf(finalRad) };
				spawnConfigs.push_back({ myPos + dir * offsetDistance, finalRad, false });
			}
		}
	}

	for (const auto& config : spawnConfigs)
	{
		GameObject* pAoEObj = PoolManager::GetInstance()->Spawn<GameObject>(poolKey);
		if (!pAoEObj)
		{
			std::cout << "[SkillComponent] Error: Failed to spawn AoE prefab from pool: " << poolKey << std::endl;
			continue;
		}

		pAoEObj->transform.SetPosition(config.position.x, config.position.y);
		pAoEObj->transform.SetRotation(config.rotation);

		if (ColliderComponent* pCol = pAoEObj->GetComponent<ColliderComponent>())
		{
			if (b2Body_IsValid(pCol->GetBodyId()))
			{
				b2Body_SetTransform(pCol->GetBodyId(), { PixelToMeter(config.position.x), PixelToMeter(config.position.y) }, b2Rot_identity);
			}
		}

		AoEComponent* pAoE = pAoEObj->GetComponent<AoEComponent>();
		if (!pAoE)
		{
			pAoE = pAoEObj->AddComponent<AoEComponent>();
		}
		if (pAoE)
		{
			pAoE->Init(data, instance.pSO.get(), &gameObject, poolKey, config.flipX);
		}
	}
}
