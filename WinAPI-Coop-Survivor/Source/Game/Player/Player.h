#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/ObserverPtr.h"
#include "Game/Interface/IDamageable.h"

class Player : public ScriptComponent, public IDamageable
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
	virtual void OnDestroy() override;
	virtual void OnCollision(ColliderComponent* other) override;

	virtual void TakeDamage(float damage, GameObject* pAttacker = nullptr) override;
	virtual bool IsDead() const override { return m_currentHP <= 0.0f; }

	void SetSpeed(float speed) { m_Speed = speed; }
	float GetSpeed() const { return m_Speed; }

	float GetCurrentHP() const { return m_currentHP; }
	float GetMaxHP() const { return m_maxHP; }
	float GetHPRatio() const { return (m_maxHP > 0.0f) ? (m_currentHP / m_maxHP) : 0.0f; }

	void SyncHP(float hp)
	{
		m_currentHP = hp;
		if (m_currentHP <= 0.0f) m_currentHP = 0.0f;
	}

private:
	void CreateTestHPBar();
	void UpdateHPBar();
	void UpdateExpGemMagnet(float dt);

private:
	float m_Speed = 500.0f;
	ObserverPtr<ColliderComponent> m_pCollider;

	float m_maxHP = 100.0f;
	float m_currentHP = 100.0f;
	float m_iFrameTimer = 0.0f;
	static constexpr float k_iFrameDuration = 0.3f;

	ObserverPtr<GameObject> m_pHpBarBgObj;
	ObserverPtr<GameObject> m_pHpBarFillObj;
	ObserverPtr<class UIImageComponent> m_pHpBarFillImg;
};
