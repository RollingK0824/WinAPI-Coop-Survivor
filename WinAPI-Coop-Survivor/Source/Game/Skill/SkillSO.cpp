#include "Engine/Core/pch.h"
#include "SkillSO.h"

SkillSO::SkillSO()
{
	ExposeVariable("SkillID", &m_skillID);
	ExposeVariable("SkillName", &m_skillName);
	ExposeVariable("Cooldown", &m_cooldown);
	ExposeVariable("Damage", &m_damage);
	ExposeVariable("Speed", &m_speed);
	ExposeVariable("Range", &m_range);
	ExposeVariable("PenetrationCount", &m_penetrationCount);
	ExposeVariable("ProjectileCount", &m_projectileCount);
	ExposeVariable("PrefabKey", &m_prefabKey);
}

void SkillSO::OnLoadFromJson(const json& j)
{
	ScriptableObject::OnLoadFromJson(j);

	if (j.contains("SkillID")) m_skillID = j["SkillID"].get<uint32>();
	if (j.contains("SkillName")) m_skillName = j["SkillName"].get<std::string>();
	if (j.contains("Category")) m_category = static_cast<ESkillCategory>(j["Category"].get<uint8>());
	if (j.contains("Cooldown")) m_cooldown = j["Cooldown"].get<float>();
	if (j.contains("Damage")) m_damage = j["Damage"].get<float>();
	if (j.contains("Speed")) m_speed = j["Speed"].get<float>();
	if (j.contains("Range")) m_range = j["Range"].get<float>();
	if (j.contains("PenetrationCount")) m_penetrationCount = j["PenetrationCount"].get<int32>();
	if (j.contains("ProjectileCount")) m_projectileCount = j["ProjectileCount"].get<int32>();
	if (j.contains("PrefabKey")) m_prefabKey = j["PrefabKey"].get<std::string>();
}

void SkillSO::OnSaveToJson(json& j) const
{
	j["SkillID"] = m_skillID;
	j["SkillName"] = m_skillName;
	j["Category"] = static_cast<uint8>(m_category);
	j["Cooldown"] = m_cooldown;
	j["Damage"] = m_damage;
	j["Speed"] = m_speed;
	j["Range"] = m_range;
	j["PenetrationCount"] = m_penetrationCount;
	j["ProjectileCount"] = m_projectileCount;
	j["PrefabKey"] = m_prefabKey;
}
