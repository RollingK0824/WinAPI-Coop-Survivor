#pragma once
#include "Engine/Framework/Base/ScriptableObject.h"

enum class ESkillCategory : uint8
{
	Projectile = 0,
	Aura,
	GroundArea
};

struct SkillLevelData
{
	int32 level = 1;
	std::string description = "";
	float damage = 20.0f;
	float cooldown = 1.0f;
	float speed = 600.0f;
	float range = 800.0f;
	float duration = 0.5f;
	int32 penetrationCount = 1;
	int32 projectileCount = 1;
};

class SkillSO : public ScriptableObject
{
public:
	SkillSO();
	virtual ~SkillSO() override = default;

	virtual void OnLoadFromJson(const json& j) override;
	virtual void OnSaveToJson(json& j) const override;
	virtual std::string GetSOTypeName() const override { return "SkillSO"; }

	uint32 GetSkillID() const { return m_skillID; }
	const std::string& GetSkillName() const { return m_skillName; }
	ESkillCategory GetCategory() const { return m_category; }
	const std::string& GetPrefabKey() const { return m_prefabKey; }

	const SkillLevelData& GetLevelData(int32 level) const;
	int32 GetMaxLevel() const { return m_levelTable.empty() ? 1 : static_cast<int32>(m_levelTable.size()); }

	float GetCooldown() const { return m_levelTable.empty() ? m_cooldown : m_levelTable[0].cooldown; }
	float GetDamage() const { return m_levelTable.empty() ? m_damage : m_levelTable[0].damage; }
	float GetSpeed() const { return m_levelTable.empty() ? m_speed : m_levelTable[0].speed; }
	float GetRange() const { return m_levelTable.empty() ? m_range : m_levelTable[0].range; }
	float GetDuration() const { return m_levelTable.empty() ? m_duration : m_levelTable[0].duration; }
	int32 GetPenetrationCount() const { return m_levelTable.empty() ? m_penetrationCount : m_levelTable[0].penetrationCount; }
	int32 GetProjectileCount() const { return m_levelTable.empty() ? m_projectileCount : m_levelTable[0].projectileCount; }

private:
	uint32 m_skillID = 1;
	std::string m_skillName = "DefaultSkill";
	ESkillCategory m_category = ESkillCategory::Projectile;
	std::string m_prefabKey = "DefaultProjectile";

	float m_cooldown = 1.0f;
	float m_damage = 20.0f;
	float m_speed = 600.0f;
	float m_range = 800.0f;
	float m_duration = 0.5f;
	int32 m_penetrationCount = 1;
	int32 m_projectileCount = 1;

	std::vector<SkillLevelData> m_levelTable;
	static SkillLevelData s_dummyLevelData;
};
