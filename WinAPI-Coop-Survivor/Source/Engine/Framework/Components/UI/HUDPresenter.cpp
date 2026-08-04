#include "Engine/Core/pch.h"
#include "HUDPresenter.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/UITextComponent.h"

static ComponentRegistrar<HUDPresenter> registrar(EngineKey::Component::HUDPresenter.data());

HUDPresenter::HUDPresenter(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
}

void HUDPresenter::Awake()
{
	m_pTextView = gameObject.GetComponent<UITextComponent>();
}

void HUDPresenter::SetData(const std::string& title, 
	const std::vector<std::pair<std::string, std::string>>& data)
{
	if (!m_pTextView || !gameObject.IsActive() || !m_bIsEnabled) return;

	std::wstring resultText = L"[" + std::wstring(title.begin(), title.end()) + L"]\n";

	for (const auto& [key, value] : data)
	{
		std::wstring wKey(key.begin(), key.end());
		std::wstring wValue(value.begin(), value.end());

		resultText += std::format(L"{}: {}\n", wKey, wValue);
	}

	m_pTextView->SetText(resultText);
}
