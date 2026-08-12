#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/ObserverPtr.h"

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

	void Init(float damage, float range, float duration, const Vector2& spawnPos, GameObject* pAttacker = nullptr);

private:
	void ApplyExplosionDamage();

private:
	float m_damage = 40.0f;
	float m_range = 120.0f;
	float m_duration = 0.5f;
	float m_lifeTimer = 0.0f;
	bool m_hasAppliedDamage = false;

	ObserverPtr<GameObject> m_pAttacker;
};
