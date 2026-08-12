#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Game/Skill/SkillSO.h"

class GameObject;

struct SkillInstance
{
	std::shared_ptr<const SkillSO> pSO = nullptr;
	int32 level = 1;
	float cooldownTimer = 0.0f;

	const SkillLevelData& GetCurrentLevelData() const
	{
		if (pSO) return pSO->GetLevelData(level);
		static SkillLevelData dummy;
		return dummy;
	}
};

class SkillComponent : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(SkillComponent)

	SkillComponent(GameObject* owner, TransformComponent* transform);
	virtual ~SkillComponent() override = default;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::SkillComponent;
	}

	virtual void Start() override;
	virtual void FixedUpdate(float fixedDt) override;

	void AddSkill(uint32 skillAssetID);
	void AddSkill(std::shared_ptr<const SkillSO> pSkillSO);
	void UpgradeSkill(uint32 skillAssetID);
	bool HasSkill(uint32 skillAssetID) const;
	int32 GetSkillLevel(uint32 skillAssetID) const;

	const std::vector<SkillInstance>& GetSkills() const { return m_skills; }

	GameObject* FindClosestMonster(float maxRange) const;

private:
	void CastSkill(SkillInstance& instance, GameObject* pTargetMonster);
	void CastProjectileSkill(const SkillInstance& instance, const SkillLevelData& data, GameObject* pTargetMonster);
	void CastAuraSkill(const SkillInstance& instance, const SkillLevelData& data);
	void CastGroundAreaSkill(const SkillInstance& instance, const SkillLevelData& data, GameObject* pTargetMonster);

private:
	int32 m_defaultSkillID = 301;
	std::vector<SkillInstance> m_skills;
};
