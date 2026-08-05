// Source/Engine/Framework/Components/UI/UIButtonComponent.h
#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include <functional>

class UIImageComponent;

class UIButtonComponent : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(UIButtonComponent)

	UIButtonComponent(GameObject* owner, TransformComponent* transform);
	virtual ~UIButtonComponent() override = default;

	void SetOnClick(std::function<void()> onClick) { m_onClick = onClick; }
	bool IsHovered() const { return m_bIsHovered; }

	virtual void Awake() override;
	virtual void Update(float dt) override;

	virtual void Serialize(json& outJson) const override;
	virtual void Deserialize(const json& inJson) override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::UIButtonComponent;
	}

private:
	bool CheckMouseOver();

private:
	UIImageComponent* m_pImgView = nullptr;
	std::function<void()> m_onClick = nullptr;
	bool m_bIsHovered = false;
};