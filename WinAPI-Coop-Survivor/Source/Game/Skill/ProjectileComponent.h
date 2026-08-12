#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/ObserverPtr.h"
#include <unordered_set>

class GameObject;
class ColliderComponent;

class ProjectileComponent : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(ProjectileComponent)

	ProjectileComponent(GameObject* owner, TransformComponent* transform);
	virtual ~ProjectileComponent() override = default;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::ProjectileComponent;
	}

	virtual void FixedUpdate(float fixedDt) override;
	virtual void OnCollision(ColliderComponent* pOtherCollider) override;

	void Init(const Vector2& dir, float speed, float damage, int32 penetration, float range, GameObject* attacker = nullptr, const std::string& poolKey = "ProjectilePool");

	void SetPoolKey(const std::string& key) { m_poolKey = key; }
	const std::string& GetPoolKey() const { return m_poolKey; }

private:
	Vector2 m_direction = { 1.0f, 0.0f };
	float m_speed = 600.0f;
	float m_damage = 20.0f;
	float m_range = 800.0f;
	float m_traveledDistance = 0.0f;
	int32 m_penetrationCount = 1;

	std::string m_poolKey = "ProjectilePool";
	ObserverPtr<GameObject> m_pAttacker;
	std::unordered_set<uint32> m_hitMonsterIDs;
};
