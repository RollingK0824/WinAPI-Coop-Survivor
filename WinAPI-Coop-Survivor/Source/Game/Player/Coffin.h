#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/ObserverPtr.h"

class Player;
class SpriteRendererComponent;

class Coffin : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(Coffin)

	Coffin(GameObject* owner, TransformComponent* transform);
	virtual ~Coffin() override = default;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::Coffin;
	}

	virtual void Start() override;
	virtual void Update(float dt) override;

	void SetTargetPlayer(Player* pPlayer) { m_pTargetPlayer = pPlayer; }
	Player* GetTargetPlayer() const { return m_pTargetPlayer.Get(); }

	float GetRezProgress() const { return (m_maxRezTime > 0.0f) ? (m_rezTimer / m_maxRezTime) : 0.0f; }

private:
	void UpdateProgressBar();

private:
	ObserverPtr<Player> m_pTargetPlayer = nullptr;
	ObserverPtr<SpriteRendererComponent> m_pBarSpriteRenderer = nullptr;

	float m_rezTimer = 0.0f;
	float m_maxRezTime = 10.0f;
};
