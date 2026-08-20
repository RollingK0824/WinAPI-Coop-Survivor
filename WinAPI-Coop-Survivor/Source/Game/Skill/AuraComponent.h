#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/ObserverPtr.h"
#include "Game/Skill/SkillSO.h"

class GameObject;

class AuraComponent : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(AuraComponent)

	AuraComponent(GameObject* owner, TransformComponent* transform);
	virtual ~AuraComponent() override = default;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::AuraComponent;
	}

	virtual void FixedUpdate(float fixedDt) override;

	void Init(const SkillLevelData& data, const SkillSO* pSO, GameObject* pCaster = nullptr, const std::string& poolKey = "GenericAuraPrefab");

	void SetPoolKey(const std::string& key) { m_poolKey = key; }
	const std::string& GetPoolKey() const { return m_poolKey; }

private:
	void ApplyAreaDamage();

private:
	float m_damage = 10.0f;
	float m_range = 150.0f;
	float m_duration = 0.0f;
	float m_lifeTimer = 0.0f;

	float m_tickInterval = 0.5f;
	float m_tickTimer = 0.0f;
	float m_rotSpeed = 2.5f;

	std::string m_poolKey = "GenericAuraPrefab";
	ObserverPtr<GameObject> m_pCaster;
};