#include "Engine/Core/pch.h"
#include "UIPanelComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/ResourceManager.h"
#include "Engine/Manager/InputManager.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/GameObject.h"

static ComponentRegistrar<UIPanelComponent> registrar(EngineKey::Component::UIPanelComponent.data());

UIPanelComponent::UIPanelComponent(GameObject* owner, TransformComponent* transform)
	: RenderComponent(owner, transform)
{
	ExposeTexture("SpriteKey", &m_spriteKey);
	ExposeVariable("Size", &m_size);
	ExposeVariable("Border", &m_RenderCommand.bitmap.sprite.border);
	ExposeVariable("RenderBackground", &m_bRenderBackground);
}

void UIPanelComponent::Awake()
{
	RenderComponent::Awake();
	auto it = std::find(s_allPanels.begin(), s_allPanels.end(), this);
	if (it == s_allPanels.end())
	{
		s_allPanels.push_back(this);
	}
}

void UIPanelComponent::OnDestroy()
{
	auto it = std::find(s_allPanels.begin(), s_allPanels.end(), this);
	if (it != s_allPanels.end())
	{
		s_allPanels.erase(it);
	}
	RenderComponent::OnDestroy();
}

bool UIPanelComponent::CheckMouseOver() const
{
	Vector2 mousePos = InputManager::GetInstance()->GetMousePosition();
	Vector2 pos = transform.GetWorldPosition();
	Vector2 scale = transform.GetWorldScale();
	Vector2 size = m_size;
	D2D1_POINT_2F pivot = m_RenderCommand.pivot;

	float finalW = size.x * scale.x;
	float finalH = size.y * scale.y;

	float left = pos.x - finalW * pivot.x;
	float right = pos.x + finalW * (1.0f - pivot.x);
	float top = pos.y - finalH * pivot.y;
	float bottom = pos.y + finalH * (1.0f - pivot.y);

	return (mousePos.x >= left && mousePos.x <= right &&
			mousePos.y >= top && mousePos.y <= bottom);
}

void UIPanelComponent::SetSpriteKey(const std::wstring& spriteKey)
{
	m_spriteKey = spriteKey;
	const Sprite* pSprite = ResourceManager::GetInstance()->GetSprite(spriteKey);
	if (pSprite != nullptr && pSprite->pTexture != nullptr)
	{
		D2D1_RECT_F oldBorder = m_RenderCommand.bitmap.sprite.border;

		m_RenderCommand.type = RenderType::BITMAP;
		m_RenderCommand.isUI = true;
		m_RenderCommand.bitmap.sprite = *pSprite;

		if (oldBorder.left != 0.0f || oldBorder.top != 0.0f || oldBorder.right != 0.0f || oldBorder.bottom != 0.0f)
		{
			m_RenderCommand.bitmap.sprite.border = oldBorder;
		}
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