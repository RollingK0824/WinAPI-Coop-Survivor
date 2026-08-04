#include "Engine/Core/pch.h"
#include "UIPanelComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Renderer/RenderCommand.h"

static ComponentRegistrar<UIPanelComponent> registrar(EngineKey::Component::UIPanelComponent.data());

UIPanelComponent::UIPanelComponent(GameObject* owner, TransformComponent* transform):Component(owner,transform)
{
}

void UIPanelComponent::AddChildUI(GameObject* pChildUI)
{
	if (pChildUI)
	{
		m_vChildUIObjects.push_back(pChildUI);
	}
}

void UIPanelComponent::RenderUI()
{

}

void UIPanelComponent::OnEnable()
{
	for (auto* child : m_vChildUIObjects)
	{
		if (child) child->SetActive(true);
	}
}

void UIPanelComponent::OnDisable()
{
	for (auto* child : m_vChildUIObjects)
	{
		if (child) child->SetActive(false);
	}
}