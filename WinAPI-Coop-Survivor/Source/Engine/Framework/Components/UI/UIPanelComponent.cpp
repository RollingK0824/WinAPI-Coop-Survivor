// Source/Engine/Framework/Components/UI/UIPanelComponent.cpp
#include "Engine/Core/pch.h"
#include "UIPanelComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/ResourceManager.h"

static ComponentRegistrar<UIPanelComponent> registrar(EngineKey::Component::UIPanelComponent.data());

UIPanelComponent::UIPanelComponent(GameObject* owner, TransformComponent* transform)
	: RenderComponent(owner, transform)
{
	m_RenderCommand.type = RenderType::RECT;
	m_RenderCommand.isUI = true;
	m_RenderCommand.zOrder = 9999;
	m_RenderCommand.color = D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.6f);

	ExposeTexture("TextureKey", &m_textureKey);
	ExposeVariable("Size", &m_size);
	ExposeVariable("RenderBackground", &m_bRenderBackground);
}

void UIPanelComponent::SetTextureKey(const std::wstring& textureKey)
{
	m_textureKey = textureKey;
	m_RenderCommand.bitmap.pTexture = ResourceManager::GetInstance()->GetTexture(textureKey);
	if (m_RenderCommand.bitmap.pTexture != nullptr)
	{
		m_RenderCommand.type = RenderType::BITMAP;
	}
	else
	{
		m_RenderCommand.type = RenderType::RECT;
	}
}

void UIPanelComponent::PostDeserialize(Scene* pScene)
{
	RenderComponent::PostDeserialize(pScene);

	m_RenderCommand.isUI = true;
	if (!m_textureKey.empty())
	{
		SetTextureKey(m_textureKey);
	}
	else
	{
		m_RenderCommand.type = RenderType::RECT;
	}
}