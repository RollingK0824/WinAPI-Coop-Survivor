#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include <functional>
#include <string>

class UITextComponent;
class UIButtonComponent;

class UIInputFieldComponent : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(UIInputFieldComponent)

	UIInputFieldComponent(GameObject* owner, TransformComponent* transform);
	virtual ~UIInputFieldComponent() override = default;

	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update(float dt) override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::UIInputFieldComponent;
	}

	const std::wstring& GetWText() const { return m_wtext; }
	std::string GetText() const;

	void SetText(const std::wstring& wtext);
	void SetText(const std::string& text);
	int GetIntValue(int defaultValue = 0) const;

	void SetFocused(bool focused);
	bool IsFocused() const { return m_bIsFocused; }

	void SetOnSubmit(std::function<void(const std::wstring&)> onSubmit) { m_onSubmit = onSubmit; }

	UITextComponent* textDisplay = nullptr;
	UIButtonComponent* clickAreaBtn = nullptr;

private:
	void ProcessKeyboardInput();
	void UpdateTextDisplay();

private:
	std::wstring m_wtext = L"";
	int m_maxLengthInt = 32;
	bool m_allowDot = true;
	bool m_digitsOnly = false;
	bool m_bIsFocused = false;

	float m_cursorBlinkTimer = 0.0f;
	bool m_showCursor = true;

	std::function<void(const std::wstring&)> m_onSubmit = nullptr;
};
