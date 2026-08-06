#pragma once
#include "Engine/Framework/Base/ScriptableObject.h"

class MonsterSO : public ScriptableObject
{
public:
	MonsterSO();
	virtual ~MonsterSO() override = default;

	virtual void OnLoadFromJson(const json& j) override;
	virtual void OnSaveToJson(json& j) const override;

	float GetMaxHP() const { return m_maxHP; }
	float GetMoveSpeed() const { return m_moveSpeed; }
	float GetAttackDamage() const { return m_attackDamage; }
	float GetColliderRadius() const { return m_colliderRadius; }
	int32 GetExpAmount() const { return m_expAmount; }
	const std::wstring& GetSpriteKey() const { return m_spriteKey; }
	const std::wstring& GetTextureKey() const { return m_spriteKey; }

private:
	float m_maxHP = 100.0f;
	float m_moveSpeed = 100.0f;
	float m_attackDamage = 10.0f;
	float m_colliderRadius = 20.0f;
	int32 m_expAmount = 10;
	std::wstring m_spriteKey = L"";
};
