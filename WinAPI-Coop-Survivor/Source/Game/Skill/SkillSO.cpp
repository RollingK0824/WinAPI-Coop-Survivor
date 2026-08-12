#include "Engine/Core/pch.h"
#include "SkillSO.h"
#include "Engine/Core/Define.h"
#include "Engine/Core/ScriptableObjectRegister.h"

static SORegistrar<SkillSO> soRegistrar(EngineKey::ScriptableObject::SkillSO.data());

SkillLevelData SkillSO::s_dummyLevelData{};

SkillSO::SkillSO()
{
	ExposeVariable("SkillID", &m_skillID);
	ExposeVariable("SkillName", &m_skillName);
	ExposeVariable("Cooldown", &m_cooldown);
	ExposeVariable("Damage", &m_damage);
	ExposeVariable("Speed", &m_speed);
	ExposeVariable("Range", &m_range);
	ExposeVariable("Duration", &m_duration);
	ExposeVariable("PenetrationCount", &m_penetrationCount);
	ExposeVariable("ProjectileCount", &m_projectileCount);
	ExposeVariable("PrefabKey", &m_prefabKey);
}

const SkillLevelData& SkillSO::GetLevelData(int32 level) const
{
	if (m_levelTable.empty())
	{
		s_dummyLevelData.level = 1;
		s_dummyLevelData.description = m_skillName;
		s_dummyLevelData.damage = m_damage;
		s_dummyLevelData.cooldown = m_cooldown;
		s_dummyLevelData.speed = m_speed;
		s_dummyLevelData.range = m_range;
		s_dummyLevelData.duration = m_duration;
		s_dummyLevelData.penetrationCount = m_penetrationCount;
		s_dummyLevelData.projectileCount = m_projectileCount;
		return s_dummyLevelData;
	}

	int32 idx = (std::max)(0, (std::min)(level - 1, static_cast<int32>(m_levelTable.size()) - 1));
	return m_levelTable[idx];
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
	if (j.contains("Duration")) m_duration = j["Duration"].get<float>();
	if (j.contains("PenetrationCount")) m_penetrationCount = j["PenetrationCount"].get<int32>();
	if (j.contains("ProjectileCount")) m_projectileCount = j["ProjectileCount"].get<int32>();
	if (j.contains("PrefabKey")) m_prefabKey = j["PrefabKey"].get<std::string>();

	m_levelTable.clear();
	if (j.contains("Levels") && j["Levels"].is_array())
	{
		for (const auto& item : j["Levels"])
		{
			SkillLevelData data;
			if (item.contains("Level")) data.level = item["Level"].get<int32>();
			if (item.contains("Description")) data.description = item["Description"].get<std::string>();
			if (item.contains("Damage")) data.damage = item["Damage"].get<float>();
			if (item.contains("Cooldown")) data.cooldown = item["Cooldown"].get<float>();
			if (item.contains("Speed")) data.speed = item["Speed"].get<float>();
			if (item.contains("Range")) data.range = item["Range"].get<float>();
			if (item.contains("Duration")) data.duration = item["Duration"].get<float>();
			if (item.contains("PenetrationCount")) data.penetrationCount = item["PenetrationCount"].get<int32>();
			if (item.contains("ProjectileCount")) data.projectileCount = item["ProjectileCount"].get<int32>();

			m_levelTable.push_back(data);
		}
	}
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
	j["Duration"] = m_duration;
	j["PenetrationCount"] = m_penetrationCount;
	j["ProjectileCount"] = m_projectileCount;
	j["PrefabKey"] = m_prefabKey;

	if (!m_levelTable.empty())
	{
		json levelsArray = json::array();
		for (const auto& data : m_levelTable)
		{
			json lvlJson;
			lvlJson["Level"] = data.level;
			lvlJson["Description"] = data.description;
			lvlJson["Damage"] = data.damage;
			lvlJson["Cooldown"] = data.cooldown;
			lvlJson["Speed"] = data.speed;
			lvlJson["Range"] = data.range;
			lvlJson["Duration"] = data.duration;
			lvlJson["PenetrationCount"] = data.penetrationCount;
			lvlJson["ProjectileCount"] = data.projectileCount;
			levelsArray.push_back(lvlJson);
		}
		j["Levels"] = levelsArray;
	}
}
