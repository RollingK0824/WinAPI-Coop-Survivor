#include "Engine/Core/pch.h"
#include "UIInputFieldComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/InputManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/UITextComponent.h"
#include "Engine/Framework/Components/UI/UIButtonComponent.h"

static ComponentRegistrar<UIInputFieldComponent> registrar(EngineKey::Component::UIInputFieldComponent.data());

UIInputFieldComponent::UIInputFieldComponent(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeVariable("Max Length", &m_maxLengthInt);
	ExposeVariable("Allow Dot", &m_allowDot);
	ExposeVariable("Digits Only", &m_digitsOnly);
	ExposeComponent("Text Display", &textDisplay);
	ExposeComponent("Click Area Button", &clickAreaBtn);
}

void UIInputFieldComponent::Awake()
{
	ScriptComponent::Awake();
}

void UIInputFieldComponent::Start()
{
	ScriptComponent::Start();

	if (!textDisplay)
	{
		textDisplay = gameObject.GetComponent<UITextComponent>();
	}

	if (!clickAreaBtn)
	{
		clickAreaBtn = gameObject.GetComponent<UIButtonComponent>();
	}

	if (clickAreaBtn)
	{
		clickAreaBtn->SetOnClick([this]() {
			SetFocused(true);
		});
	}
	else
	{
		SetFocused(true);
	}

	UpdateTextDisplay();
}

void UIInputFieldComponent::SetText(const std::wstring& wtext)
{
	m_wtext = wtext;
	UpdateTextDisplay();
}

void UIInputFieldComponent::SetText(const std::string& text)
{
	m_wtext = std::wstring(text.begin(), text.end());
	UpdateTextDisplay();
}

std::string UIInputFieldComponent::GetText() const
{
	return std::string(m_wtext.begin(), m_wtext.end());
}

int UIInputFieldComponent::GetIntValue(int defaultValue) const
{
	if (m_wtext.empty()) return defaultValue;
	try {
		return std::stoi(m_wtext);
	}
	catch (...) {
		return defaultValue;
	}
}

void UIInputFieldComponent::SetFocused(bool focused)
{
	m_bIsFocused = focused;
	m_cursorBlinkTimer = 0.0f;
	m_showCursor = focused;
	UpdateTextDisplay();
}

void UIInputFieldComponent::Update(float dt)
{
	ScriptComponent::Update(dt);

	InputManager* input = InputManager::GetInstance();
	if (!input) return;

	if (input->GetKeyDown(VK_LBUTTON))
	{
		if (clickAreaBtn)
		{
			if (clickAreaBtn->IsHovered())
			{
				SetFocused(true);
			}
			else
			{
				SetFocused(false);
			}
		}
	}

	if (!m_bIsFocused) return;


	m_cursorBlinkTimer += dt;
	if (m_cursorBlinkTimer >= 0.5f)
	{
		m_cursorBlinkTimer = 0.0f;
		m_showCursor = !m_showCursor;
		UpdateTextDisplay();
	}

	ProcessKeyboardInput();
}

void UIInputFieldComponent::ProcessKeyboardInput()
{
	InputManager* input = InputManager::GetInstance();
	if (!input) return;

	size_t maxLen = static_cast<size_t>(m_maxLengthInt > 0 ? m_maxLengthInt : 32);

	const auto& inputChars = input->GetInputChars();
	bool handledBackspace = false;

	for (wchar_t ch : inputChars)
	{
		if (ch == L'\b')
		{
			handledBackspace = true;
			if (!m_wtext.empty())
			{
				m_wtext.pop_back();
				UpdateTextDisplay();
			}
		}
		else if (ch == L'\r' || ch == L'\n')
		{
			if (m_onSubmit)
			{
				m_onSubmit(m_wtext);
			}
		}
		else if (ch >= 32)
		{
			if (m_digitsOnly)
			{
				if (ch < L'0' || ch > L'9') continue;
			}

			if (m_wtext.size() < maxLen)
			{
				m_wtext += ch;
				UpdateTextDisplay();
			}
		}
	}

	if (!handledBackspace && input->GetKeyDown(VK_BACK))
	{
		if (!m_wtext.empty())
		{
			m_wtext.pop_back();
			UpdateTextDisplay();
		}
	}

	if (input->GetKeyDown(VK_RETURN) && inputChars.empty())
	{
		if (m_onSubmit)
		{
			m_onSubmit(m_wtext);
		}
	}
}


void UIInputFieldComponent::UpdateTextDisplay()
{
	if (!textDisplay) return;

	std::wstring displayText = m_wtext;
	if (m_bIsFocused && m_showCursor)
	{
		displayText += L"|";
	}

	textDisplay->SetText(displayText);
}
