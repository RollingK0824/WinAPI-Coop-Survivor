// Source/Engine/Framework/Components/UI/UIButtonComponent.h
#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include <functional>

class UIImageComponent;
class UIPanelComponent;

class UIButtonComponent : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(UIButtonComponent)

	UIButtonComponent(GameObject* owner, TransformComponent* transform);
	virtual ~UIButtonComponent() override = default;

	virtual void Awake() override;
	virtual void Update(float dt) override;

	void SetOnClick(std::function<void()> onClick) { m_onClick = onClick; }
	bool IsHovered() const { return m_bIsHovered; }

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::UIButtonComponent;
	}

private:
	bool CheckMouseOver();

	UIImageComponent* m_pImgView = nullptr;
	UIPanelComponent* m_pPanelView = nullptr;
	std::function<void()> m_onClick = nullptr;
	bool m_bIsHovered = false;
};