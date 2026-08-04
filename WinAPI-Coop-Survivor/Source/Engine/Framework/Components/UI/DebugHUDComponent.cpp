#include "Engine/Core/pch.h"
#include "DebugHUDComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/DebugManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/HUDPresenter.h"

static ComponentRegistrar<DebugHUDComponent> registrar(EngineKey::Component::DebugHUDComponent.data());

DebugHUDComponent::DebugHUDComponent(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner,transform)
{
}

void DebugHUDComponent::Awake()
{
	m_pPresenter = gameObject.GetComponent<HUDPresenter>();

	if (!m_pPresenter)return;

	DebugManager::GetInstance()->RegisterDebugHUD(this, &gameObject);
}

void DebugHUDComponent::OnDestroy()
{
	ScriptComponent::OnDestroy();

	DebugManager::GetInstance()->UnRegisterDebugHUD(this);
}

void DebugHUDComponent::UpdateDebugData(const std::string& title, 
	const std::vector<std::pair<std::string, std::string>>& data)
{
	if (m_pPresenter)
	{
		m_pPresenter->SetData(title, data);
	}
}
