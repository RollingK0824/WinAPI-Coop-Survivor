#pragma once
#include "Engine/Framework/Base/ScriptableObject.h"

enum class ESkillCategory : uint8
{
	Projectile = 0,
	Aura,
	GroundArea
};

class SkillSO : public ScriptableObject
{
public:
	SkillSO();
	virtual ~SkillSO() override = default;

	virtual void OnLoadFromJson(const json& j) override;
	virtual void OnSaveToJson(json& j) const override;

	uint32 GetSkillID() const { return m_skillID; }
	const std::string& GetSkillName() const { return m_skillName; }
	ESkillCategory GetCategory() const { return m_category; }
	float GetCooldown() const { return m_cooldown; }
	float GetDamage() const { return m_damage; }
	float GetSpeed() const { return m_speed; }
	float GetRange() const { return m_range; }
	int32 GetPenetrationCount() const { return m_penetrationCount; }
	int32 GetProjectileCount() const { return m_projectileCount; }
	const std::string& GetPrefabKey() const { return m_prefabKey; }

private:
	uint32 m_skillID = 1;
	std::string m_skillName = "DefaultSkill";
	ESkillCategory m_category = ESkillCategory::Projectile;
	float m_cooldown = 1.0f;
	float m_damage = 20.0f;
	float m_speed = 600.0f;
	float m_range = 800.0f;
	int32 m_penetrationCount = 1;
	int32 m_projectileCount = 1;
	std::string m_prefabKey = "DefaultProjectile";
};
