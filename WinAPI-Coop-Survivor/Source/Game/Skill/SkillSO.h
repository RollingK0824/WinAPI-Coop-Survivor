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
	void SetSkillID(uint32 id) { m_skillID = id; m_assetID = id; }
	const std::string& GetSkillName() const { return m_skillName; }
	void SetSkillName(const std::string& name) { m_skillName = name; m_assetName = name; }
	ESkillCategory GetCategory() const { return m_category; }
	void SetCategory(ESkillCategory cat) { m_category = cat; }
	const std::string& GetPrefabKey() const { return m_prefabKey; }
	void SetPrefabKey(const std::string& key) { m_prefabKey = key; }

	const std::wstring& GetSpriteKey() const { return m_spriteKey; }
	void SetSpriteKey(const std::wstring& key) { m_spriteKey = key; }

	const std::string& GetEffectKey() const { return m_effectKey; }
	void SetEffectKey(const std::string& key) { m_effectKey = key; }

	std::vector<SkillLevelData>& GetMutableLevelTable() { return m_levelTable; }
	const std::vector<SkillLevelData>& GetLevelTable() const { return m_levelTable; }

	const SkillLevelData& GetLevelData(int32 level) const;
	int32 GetMaxLevel() const { return m_levelTable.empty() ? 1 : static_cast<int32>(m_levelTable.size()); }

	float GetCooldown() const { return GetLevelData(1).cooldown; }
	float GetDamage() const { return GetLevelData(1).damage; }
	float GetSpeed() const { return GetLevelData(1).speed; }
	float GetRange() const { return GetLevelData(1).range; }
	float GetDuration() const { return GetLevelData(1).duration; }
	int32 GetPenetrationCount() const { return GetLevelData(1).penetrationCount; }
	int32 GetProjectileCount() const { return GetLevelData(1).projectileCount; }

private:
	uint32 m_skillID = 1;
	std::string m_skillName = "DefaultSkill";
	ESkillCategory m_category = ESkillCategory::Projectile;
	std::string m_prefabKey = "DefaultProjectile";
	std::wstring m_spriteKey = L"";
	std::string m_effectKey = "";

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
