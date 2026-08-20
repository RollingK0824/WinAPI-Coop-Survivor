#pragma once
#include "Engine/Framework/Base/ScriptableObject.h"

class MonsterSO : public ScriptableObject
{
public:
	MonsterSO();
	virtual ~MonsterSO() override = default;

	virtual void OnLoadFromJson(const json& j) override;
	virtual void OnSaveToJson(json& j) const override;
	virtual std::string GetSOTypeName() const override { return "MonsterSO"; }

	float GetMaxHP() const { return m_maxHP; }
	float GetMoveSpeed() const { return m_moveSpeed; }
	float GetAttackDamage() const { return m_attackDamage; }
	float GetColliderRadius() const { return m_colliderRadius; }
	int32 GetExpAmount() const { return m_expAmount; }
	const std::wstring& GetSpriteKey() const { return m_spriteKey; }
	const std::wstring& GetTextureKey() const { return m_spriteKey; }
	const std::string& GetAnimClipKey() const { return m_animClipKey; }
	const std::string& GetDieClipKey() const { return m_dieClipKey; }

	void SetMaxHP(float hp) { m_maxHP = hp; }
	void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
	void SetAttackDamage(float dmg) { m_attackDamage = dmg; }
	void SetColliderRadius(float r) { m_colliderRadius = r; }
	void SetExpAmount(int32 exp) { m_expAmount = exp; }
	void SetSpriteKey(const std::wstring& key) { m_spriteKey = key; }
	void SetAnimClipKey(const std::string& key) { m_animClipKey = key; }
	void SetDieClipKey(const std::string& key) { m_dieClipKey = key; }

private:
	float m_maxHP = 100.0f;
	float m_moveSpeed = 100.0f;
	float m_attackDamage = 10.0f;
	float m_colliderRadius = 20.0f;
	int32 m_expAmount = 10;
	std::wstring m_spriteKey = L"";
	std::string m_animClipKey = "";
	std::string m_dieClipKey = "";
};
