#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/ObserverPtr.h"

class Player : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(Player)

	Player(GameObject* owner, TransformComponent* transform);
	virtual ~Player() override = default;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::Player;
	}

	virtual void Start() override;
	virtual void Update(float dt) override;

	void SetSpeed(float speed) { m_Speed = speed; }
	float GetSpeed() const { return m_Speed; }

private:
	float m_Speed = 500.0f;
	ObserverPtr<ColliderComponent> m_pCollider;
};
