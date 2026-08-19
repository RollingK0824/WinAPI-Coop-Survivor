#include "Engine/Core/pch.h"
#include "SkillSO.h"
#include "Engine/Core/Define.h"
#include "Engine/Core/ScriptableObjectRegister.h"

static SORegistrar<SkillSO> soRegistrar(EngineKey::ScriptableObject::SkillSO.data());

SkillLevelData SkillSO::s_dummyLevelData{};

SkillSO::SkillSO()
{
	m_skillID = 301;
	SetAssetID(m_skillID);

	ExposeVariable("SkillID", &m_skillID);
	ExposeVariable("SkillName", &m_skillName);
	ExposeVariable("Cooldown", &m_cooldown);
	ExposeVariable("Damage", &m_damage);
	ExposeVariable("Speed", &m_speed);
	ExposeVariable("Range", &m_range);
	ExposeVariable("Duration", &m_duration);
	ExposeVariable("PenetrationCount", &m_penetrationCount);
	ExposeVariable("ProjectileCount", &m_projectileCount);
	ExposeVariable("AimType", reinterpret_cast<uint8*>(&m_aimType));
	ExposeVariable("SpreadAngle", &m_spreadAngle);
	ExposeVariable("FixedAngleDeg", &m_fixedAngleDeg);
	ExposeVariable("PrefabKey", &m_prefabKey);
	ExposeTexture("SpriteKey", &m_spriteKey);
	ExposeVariable("EffectKey", &m_effectKey);

	m_levelTable.clear();
	for (int i = 0; i < 5; ++i)
	{
		SkillLevelData lvl;
		lvl.level = i + 1;
		lvl.description = "Level " + std::to_string(i + 1) + " Skill Spec";
		lvl.damage = m_damage + i * 10.0f;
		lvl.cooldown = (std::max)(0.2f, m_cooldown - i * 0.1f);
		lvl.speed = m_speed;
		lvl.range = m_range;
		lvl.duration = m_duration;
		lvl.penetrationCount = m_penetrationCount;
		lvl.projectileCount = m_projectileCount;
		lvl.aimType = m_aimType;
		lvl.spreadAngle = m_spreadAngle;
		lvl.fixedAngleDeg = m_fixedAngleDeg;
		m_levelTable.push_back(lvl);
	}
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
		s_dummyLevelData.aimType = m_aimType;
		s_dummyLevelData.spreadAngle = m_spreadAngle;
		s_dummyLevelData.fixedAngleDeg = m_fixedAngleDeg;
		return s_dummyLevelData;
	}

	int32 targetIdx = level - 1;
	if (targetIdx < 0) targetIdx = 0;
	if (targetIdx >= static_cast<int32>(m_levelTable.size()))
	{
		targetIdx = static_cast<int32>(m_levelTable.size()) - 1;
	}

	return m_levelTable[static_cast<size_t>(targetIdx)];
}

void SkillSO::OnLoadFromJson(const json& j)
{
	ScriptableObject::OnLoadFromJson(j);

	if (j.contains("SkillID"))
	{
		m_skillID = j["SkillID"].get<uint32>();
		SetAssetID(m_skillID);
	}
	else if (GetAssetID() != 0)
	{
		m_skillID = GetAssetID();
	}

	if (j.contains("AssetName"))
	{
		m_assetName = j["AssetName"].get<std::string>();
		m_skillName = m_assetName;
	}
	if (j.contains("SkillName"))
	{
		m_skillName = j["SkillName"].get<std::string>();
		if (GetAssetName().empty()) SetAssetName(m_skillName);
	}
	if (j.contains("Category")) m_category = static_cast<ESkillCategory>(j["Category"].get<uint8>());
	if (j.contains("Cooldown")) m_cooldown = j["Cooldown"].get<float>();
	if (j.contains("Damage")) m_damage = j["Damage"].get<float>();
	if (j.contains("Speed")) m_speed = j["Speed"].get<float>();
	if (j.contains("Range")) m_range = j["Range"].get<float>();
	if (j.contains("Duration")) m_duration = j["Duration"].get<float>();
	if (j.contains("PenetrationCount")) m_penetrationCount = j["PenetrationCount"].get<int32>();
	if (j.contains("ProjectileCount")) m_projectileCount = j["ProjectileCount"].get<int32>();
	if (j.contains("AimType")) m_aimType = static_cast<EAimType>(j["AimType"].get<uint8>());
	if (j.contains("SpreadAngle")) m_spreadAngle = j["SpreadAngle"].get<float>();
	if (j.contains("FixedAngleDeg")) m_fixedAngleDeg = j["FixedAngleDeg"].get<float>();
	if (j.contains("PrefabKey")) m_prefabKey = j["PrefabKey"].get<std::string>();
	if (j.contains("EffectKey")) m_effectKey = j["EffectKey"].get<std::string>();
	if (j.contains("SpriteKey"))
	{
		std::string keyStr = j["SpriteKey"].get<std::string>();
		m_spriteKey = std::wstring(keyStr.begin(), keyStr.end());
	}

	if (j.contains("Levels") && j["Levels"].is_array() && !j["Levels"].empty())
	{
		m_levelTable.clear();
		for (const auto& item : j["Levels"])
		{
			SkillLevelData data;
			data.aimType = m_aimType;
			data.spreadAngle = m_spreadAngle;
			data.fixedAngleDeg = m_fixedAngleDeg;

			if (item.contains("Level")) data.level = item["Level"].get<int32>();
			if (item.contains("Description")) data.description = item["Description"].get<std::string>();
			if (item.contains("Damage")) data.damage = item["Damage"].get<float>();
			if (item.contains("Cooldown")) data.cooldown = item["Cooldown"].get<float>();
			if (item.contains("Speed")) data.speed = item["Speed"].get<float>();
			if (item.contains("Range")) data.range = item["Range"].get<float>();
			if (item.contains("Duration")) data.duration = item["Duration"].get<float>();
			if (item.contains("PenetrationCount")) data.penetrationCount = item["PenetrationCount"].get<int32>();
			if (item.contains("ProjectileCount")) data.projectileCount = item["ProjectileCount"].get<int32>();
			if (item.contains("AimType")) data.aimType = static_cast<EAimType>(item["AimType"].get<uint8>());
			if (item.contains("SpreadAngle")) data.spreadAngle = item["SpreadAngle"].get<float>();
			if (item.contains("FixedAngleDeg")) data.fixedAngleDeg = item["FixedAngleDeg"].get<float>();

			m_levelTable.push_back(data);
		}
	}
}

void SkillSO::OnSaveToJson(json& j) const
{
	j["AssetID"] = m_skillID;
	j["AssetName"] = m_skillName;
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
	j["AimType"] = static_cast<uint8>(m_aimType);
	j["SpreadAngle"] = m_spreadAngle;
	j["FixedAngleDeg"] = m_fixedAngleDeg;
	j["PrefabKey"] = m_prefabKey;

	std::string spriteStr(m_spriteKey.begin(), m_spriteKey.end());
	j["SpriteKey"] = spriteStr;
	j["EffectKey"] = m_effectKey;

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
		lvlJson["AimType"] = static_cast<uint8>(data.aimType);
		lvlJson["SpreadAngle"] = data.spreadAngle;
		lvlJson["FixedAngleDeg"] = data.fixedAngleDeg;
		levelsArray.push_back(lvlJson);
	}
	j["Levels"] = levelsArray;
}
