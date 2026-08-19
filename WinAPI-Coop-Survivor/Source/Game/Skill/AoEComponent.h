#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/ObserverPtr.h"
#include "Game/Skill/SkillSO.h"

class GameObject;

class AoEComponent : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(AoEComponent)

	AoEComponent(GameObject* owner, TransformComponent* transform);
	virtual ~AoEComponent() override = default;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::AoEComponent;
	}

	virtual void FixedUpdate(float fixedDt) override;

	void Init(const SkillLevelData& data, const SkillSO* pSO, GameObject* attacker = nullptr, const std::string& poolKey = "GenericAoEPrefab");

	void SetPoolKey(const std::string& key) { m_poolKey = key; }
	const std::string& GetPoolKey() const { return m_poolKey; }

private:
	void ApplyExplosionDamage();

private:
	float m_damage = 40.0f;
	float m_range = 120.0f;
	float m_duration = 0.5f;
	float m_lifeTimer = 0.0f;
	float m_tickInterval = 0.0f;
	float m_tickTimer = 0.0f;
	bool m_hasAppliedDamage = false;

	std::string m_poolKey = "GenericAoEPrefab";
	ObserverPtr<GameObject> m_pAttacker;
};
