#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/ObserverPtr.h"

class Player:public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(Player)

	Player(GameObject* owner, TransformComponent* transform);
	virtual ~Player() override {}
	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::Player;
	}

	virtual void Start() override;
	virtual void Update(float dt) override;

	void SetSpeed(float speed) { m_Speed = speed; }
	float GetSpeed()const { return m_Speed; }

	void SetNetworkInfo(unsigned int netID, bool isLocal) {
		m_NetID = netID;
		m_bIsLocal = isLocal;
	}
	bool IsLocal() const { return m_bIsLocal; }
	unsigned int GetNetID() const { return m_NetID; }

	void InitializeNetworkController(unsigned int netID, bool isLocal);

private:
	float m_Speed = 500.0f;
	unsigned int m_NetID = 0;
	bool m_bIsLocal = true;

	ObserverPtr<ColliderComponent> m_pCollider;
};

