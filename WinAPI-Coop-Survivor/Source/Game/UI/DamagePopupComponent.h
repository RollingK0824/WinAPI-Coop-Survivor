#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/ObserverPtr.h"
#include <string>

class UITextComponent;

class DamagePopupComponent : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(DamagePopupComponent)

	DamagePopupComponent(GameObject* owner, TransformComponent* transform);
	virtual ~DamagePopupComponent() override = default;

	virtual void Awake() override;
	virtual void Update(float dt) override;

	void Init(int damage, const Vector2& spawnPos, bool isCritical = false, const std::string& poolKey = "DamageTextPrefab");

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::DamagePopup;
	}

private:
	ObserverPtr<UITextComponent> m_pText;
	std::string m_poolKey = "DamageTextPrefab";
	float m_lifeTimer = 0.0f;
	float m_duration = 0.7f;
	Vector2 m_velocity = { 0.0f, -60.0f };
	float m_swayFreq = 12.0f;
	float m_swayAmp = 15.0f;
	D2D1::ColorF m_baseColor = D2D1::ColorF(D2D1::ColorF::White);
};
