#include "Engine/Core/pch.h"
#include "MonsterSO.h"

MonsterSO::MonsterSO()
{
	ExposeVariable("MaxHP", &m_maxHP);
	ExposeVariable("MoveSpeed", &m_moveSpeed);
	ExposeVariable("AttackDamage", &m_attackDamage);
	ExposeVariable("ColliderRadius", &m_colliderRadius);
	ExposeVariable("ExpAmount", &m_expAmount);
	ExposeTexture("SpriteKey", &m_spriteKey);
}

void MonsterSO::OnLoadFromJson(const json& j)
{
	ScriptableObject::OnLoadFromJson(j);

	if (j.contains("MaxHP")) m_maxHP = j["MaxHP"].get<float>();
	if (j.contains("MoveSpeed")) m_moveSpeed = j["MoveSpeed"].get<float>();
	if (j.contains("AttackDamage")) m_attackDamage = j["AttackDamage"].get<float>();
	if (j.contains("ColliderRadius")) m_colliderRadius = j["ColliderRadius"].get<float>();
	if (j.contains("ExpAmount")) m_expAmount = j["ExpAmount"].get<int32>();

	if (j.contains("SpriteKey"))
	{
		std::string keyStr = j["SpriteKey"].get<std::string>();
		m_spriteKey = std::wstring(keyStr.begin(), keyStr.end());
	}
	else if (j.contains("TextureKey"))
	{
		std::string keyStr = j["TextureKey"].get<std::string>();
		m_spriteKey = std::wstring(keyStr.begin(), keyStr.end());
	}
}

void MonsterSO::OnSaveToJson(json& j) const
{
	j["MaxHP"] = m_maxHP;
	j["MoveSpeed"] = m_moveSpeed;
	j["AttackDamage"] = m_attackDamage;
	j["ColliderRadius"] = m_colliderRadius;
	j["ExpAmount"] = m_expAmount;

	std::string keyStr(m_spriteKey.begin(), m_spriteKey.end());
	j["SpriteKey"] = keyStr;
}
