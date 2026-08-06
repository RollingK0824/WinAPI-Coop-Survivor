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

	ExposeTexture("SpriteKey", &m_spriteKey);
	ExposeVariable("Size", &m_size);
	ExposeVariable("RenderBackground", &m_bRenderBackground);
}

void UIPanelComponent::SetSpriteKey(const std::wstring& spriteKey)
{
	m_spriteKey = spriteKey;
	const Sprite* pSprite = ResourceManager::GetInstance()->GetSprite(spriteKey);
	if (pSprite != nullptr && pSprite->pTexture != nullptr)
	{
		m_RenderCommand.type = RenderType::BITMAP;
		m_RenderCommand.isUI = true;
		m_RenderCommand.bitmap.sprite = *pSprite;
	}
	else
	{
		m_RenderCommand.type = RenderType::RECT;
		m_RenderCommand.isUI = true;
	}
}

void UIPanelComponent::PostDeserialize(Scene* pScene)
{
	RenderComponent::PostDeserialize(pScene);

	m_RenderCommand.isUI = true;
	if (!m_spriteKey.empty())
	{
		SetSpriteKey(m_spriteKey);
	}
	else
	{
		m_RenderCommand.type = RenderType::RECT;
	}
}