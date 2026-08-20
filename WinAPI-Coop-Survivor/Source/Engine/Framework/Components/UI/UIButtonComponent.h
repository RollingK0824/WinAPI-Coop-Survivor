// Source/Engine/Framework/Components/UI/UIButtonComponent.h
#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class UIImageComponent;
class UIPanelComponent;

enum class ButtonVisualState
{
	Normal,
	Hover,
	Pressed,
	Disabled
};

class UIButtonComponent : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(UIButtonComponent)

	UIButtonComponent(GameObject* owner, TransformComponent* transform);
	virtual ~UIButtonComponent() override = default;

	virtual void Awake() override;
	virtual void OnDestroy() override;
	virtual void Update(float dt) override;
	virtual void PostDeserialize(Scene* pScene) override;

	void SetOnClick(std::function<void()> onClick) { m_onClick = onClick; }
	bool IsHovered() const { return m_bIsHovered; }
	bool IsPressed() const { return m_bIsPressed; }

	int GetEffectiveZOrder() const;
	bool IsBlockedByHigherPanel(int myZOrder) const;

	void SetNormalSpriteKey(const std::wstring& spriteKey) { m_normalSpriteKey = spriteKey; m_bVisualInitialized = false; }
	void SetHoverSpriteKey(const std::wstring& spriteKey) { m_hoverSpriteKey = spriteKey; m_bVisualInitialized = false; }
	void SetPressedSpriteKey(const std::wstring& spriteKey) { m_pressedSpriteKey = spriteKey; m_bVisualInitialized = false; }
	void SetDisabledSpriteKey(const std::wstring& spriteKey) { m_disabledSpriteKey = spriteKey; m_bVisualInitialized = false; }

	void SetNormalColor(const D2D1_COLOR_F& color) { m_normalColor = color; m_bVisualInitialized = false; }
	void SetHoverColor(const D2D1_COLOR_F& color) { m_hoverColor = color; m_bVisualInitialized = false; }
	void SetPressedColor(const D2D1_COLOR_F& color) { m_pressedColor = color; m_bVisualInitialized = false; }
	void SetDisabledColor(const D2D1_COLOR_F& color) { m_disabledColor = color; m_bVisualInitialized = false; }

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::UIButtonComponent;
	}

private:
	bool CheckMouseOver();
	void ApplyVisualState(ButtonVisualState state);
	static UIButtonComponent* GetTopmostHoveredButton();

private:
	static inline std::vector<UIButtonComponent*> s_allButtons;

	UIImageComponent* m_pImgView = nullptr;
	UIPanelComponent* m_pPanelView = nullptr;
	std::function<void()> m_onClick = nullptr;

	bool m_bIsHovered = false;
	bool m_bIsPressed = false;

	ButtonVisualState m_visualState = ButtonVisualState::Normal;
	bool m_bVisualInitialized = false;

	std::wstring m_normalSpriteKey = L"";
	std::wstring m_hoverSpriteKey = L"";
	std::wstring m_pressedSpriteKey = L"";
	std::wstring m_disabledSpriteKey = L"";

	D2D1_COLOR_F m_normalColor = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
	D2D1_COLOR_F m_hoverColor = D2D1::ColorF(0.9f, 0.9f, 0.9f, 1.0f);
	D2D1_COLOR_F m_pressedColor = D2D1::ColorF(0.7f, 0.7f, 0.7f, 1.0f);
	D2D1_COLOR_F m_disabledColor = D2D1::ColorF(0.5f, 0.5f, 0.5f, 1.0f);
};