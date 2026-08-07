#include "Engine/Core/pch.h"
#include "SpriteRendererComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/ResourceManager.h"

static ComponentRegistrar<SpriteRendererComponent> registrar(EngineKey::Component::SpriteRenderer.data());

SpriteRendererComponent::SpriteRendererComponent(GameObject* owner, TransformComponent* transform)
	: RenderComponent(owner, transform)
{
	m_RenderCommand.type = RenderType::BITMAP;

	ExposeTexture("SpriteKey", &m_spriteKey);
	ExposeVariable("Size", &m_size);
	ExposeVariable("Pivot", &m_RenderCommand.bitmap.sprite.pivot);
	ExposeVariable("Offset", &m_RenderCommand.bitmap.sprite.offset);
	ExposeVariable("FlipX", &m_RenderCommand.bitmap.flipX);
	ExposeVariable("FlipY", &m_RenderCommand.bitmap.flipY);
}

void SpriteRendererComponent::Awake()
{
	RenderComponent::Awake();

	if (!m_spriteKey.empty())
	{
		SetSpriteKey(m_spriteKey);
	}
}

void SpriteRendererComponent::SetAsSprite(const Sprite& sprite)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.bitmap.sprite = sprite;

	float srcW = sprite.srcRect.right - sprite.srcRect.left;
	float srcH = sprite.srcRect.bottom - sprite.srcRect.top;

	if (m_size.x <= 0.0f || m_size.y <= 0.0f)
	{
		m_size = { srcW, srcH };
	}
	m_RenderCommand.bitmap.size = m_size;
}

void SpriteRendererComponent::SetSize(Vector2 size)
{
	m_size = size;
	m_RenderCommand.bitmap.size = m_size;
}

void SpriteRendererComponent::SetSpriteKey(const std::wstring& spriteKey)
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

void SpriteRendererComponent::SetAsBitmap(ID2D1Bitmap* pBitmap, D2D1_RECT_F srcRect)
{
	m_RenderCommand.type = RenderType::BITMAP;
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

void SpriteRendererComponent::SetAsBitmap(const std::wstring& textureKey, D2D1_RECT_F srcRect)
{
	m_RenderCommand.type = RenderType::BITMAP;
	SetSpriteKey(textureKey);
	m_RenderCommand.bitmap.sprite.srcRect = srcRect;
}

void SpriteRendererComponent::SetNativeSize()
{
	const Sprite* pSprite = ResourceManager::GetInstance()->GetSprite(m_spriteKey);
	if (pSprite != nullptr)
	{
		float srcW = pSprite->srcRect.right - pSprite->srcRect.left;
		float srcH = pSprite->srcRect.bottom - pSprite->srcRect.top;
		SetSize({ srcW, srcH });
		SetAsSprite(*pSprite);
	}
	else if (m_RenderCommand.bitmap.sprite.pTexture)
	{
		D2D1_SIZE_F size = m_RenderCommand.bitmap.sprite.pTexture->GetSize();
		SetSize({ size.width, size.height });
		m_RenderCommand.bitmap.sprite.srcRect = D2D1::RectF(0.0f, 0.0f, size.width, size.height);
	}
}

Vector2 SpriteRendererComponent::GetSpriteSize() const
{
	float width = (m_size.x > 0.0f) ? m_size.x : (m_RenderCommand.bitmap.sprite.srcRect.right - m_RenderCommand.bitmap.sprite.srcRect.left);
	float height = (m_size.y > 0.0f) ? m_size.y : (m_RenderCommand.bitmap.sprite.srcRect.bottom - m_RenderCommand.bitmap.sprite.srcRect.top);

	return Vector2(width * m_RenderCommand.scaleX, height * m_RenderCommand.scaleY);
}

void SpriteRendererComponent::PostDeserialize(Scene* pScene)
{
	RenderComponent::PostDeserialize(pScene);

	m_RenderCommand.type = RenderType::BITMAP;
	if (!m_spriteKey.empty())
	{
		SetSpriteKey(m_spriteKey);
	}
	m_RenderCommand.bitmap.size = m_size;
}
