#include "Engine/Core/pch.h"
#include "UIImageComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/ResourceManager.h"

static ComponentRegistrar<UIImageComponent> registrar(EngineKey::Component::UIImageComponent.data());

UIImageComponent::UIImageComponent(GameObject* owner, TransformComponent* transform)
	: RenderComponent(owner, transform)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.isUI = true;
	m_RenderCommand.zOrder = 9999;

	ExposeTexture("SpriteKey", &m_spriteKey);
	ExposeVariable("Size", &m_size);
	ExposeVariable("Pivot", &m_RenderCommand.bitmap.sprite.pivot);
	ExposeVariable("Offset", &m_RenderCommand.bitmap.sprite.offset);
	ExposeVariable("FillAmount", &m_fillAmount);
	ExposeVariable("FlipX", &m_RenderCommand.bitmap.flipX);
	ExposeVariable("FlipY", &m_RenderCommand.bitmap.flipY);
}

void UIImageComponent::SetAsSprite(const Sprite& sprite)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.isUI = true;
	m_RenderCommand.bitmap.sprite = sprite;

	float srcW = sprite.srcRect.right - sprite.srcRect.left;
	float srcH = sprite.srcRect.bottom - sprite.srcRect.top;
	if (m_size.x <= 0.0f || m_size.y <= 0.0f)
	{
		m_size = { srcW, srcH };
	}
	m_RenderCommand.bitmap.size = m_size;
}

void UIImageComponent::SetSpriteKey(const std::wstring& spriteKey)
{
	m_spriteKey = spriteKey;
	const Sprite* pSprite = ResourceManager::GetInstance()->GetSprite(spriteKey);
	if (pSprite != nullptr)
	{
		D2D1_POINT_2F oldPivot = m_RenderCommand.bitmap.sprite.pivot;
		D2D1_POINT_2F oldOffset = m_RenderCommand.bitmap.sprite.offset;

		SetAsSprite(*pSprite);

		if (oldPivot.x != 0.0f || oldPivot.y != 0.0f)
		{
			m_RenderCommand.bitmap.sprite.pivot = oldPivot;
		}
		if (oldOffset.x != 0.0f || oldOffset.y != 0.0f)
		{
			m_RenderCommand.bitmap.sprite.offset = oldOffset;
		}
	}
}

void UIImageComponent::SetAsBitmap(ID2D1Bitmap* pBitmap, D2D1_RECT_F srcRect)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.isUI = true;
	m_RenderCommand.bitmap.sprite.pTexture = pBitmap;
	m_RenderCommand.bitmap.sprite.srcRect = srcRect;
	m_RenderCommand.bitmap.sprite.pivot = D2D1::Point2F(0.5f, 0.5f);

	float srcW = srcRect.right - srcRect.left;
	float srcH = srcRect.bottom - srcRect.top;
	if (m_size.x <= 0.0f || m_size.y <= 0.0f)
	{
		m_size = { srcW, srcH };
	}
	m_RenderCommand.bitmap.size = m_size;
}

void UIImageComponent::SetAsBitmap(const std::wstring& textureKey, D2D1_RECT_F srcRect)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.isUI = true;
	SetSpriteKey(textureKey);
	m_RenderCommand.bitmap.sprite.srcRect = srcRect;
}

void UIImageComponent::SetNativeSize()
{
	const Sprite* pSprite = ResourceManager::GetInstance()->GetSprite(m_spriteKey);
	if (pSprite != nullptr)
	{
		SetAsSprite(*pSprite);
		float width = pSprite->srcRect.right - pSprite->srcRect.left;
		float height = pSprite->srcRect.bottom - pSprite->srcRect.top;
		m_size = { width, height };
	}
	else if (m_RenderCommand.bitmap.sprite.pTexture)
	{
		D2D1_SIZE_F size = m_RenderCommand.bitmap.sprite.pTexture->GetSize();
		m_size = { size.width, size.height };
		m_RenderCommand.bitmap.sprite.srcRect = D2D1::RectF(0.0f, 0.0f, size.width, size.height);
	}
	m_RenderCommand.bitmap.size = m_size;
}

void UIImageComponent::PostDeserialize(Scene* pScene)
{
	RenderComponent::PostDeserialize(pScene);

	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.isUI = true;
	if (!m_spriteKey.empty())
	{
		SetSpriteKey(m_spriteKey);
	}
	m_RenderCommand.bitmap.size = m_size;
}