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

	ExposeTexture("TextureKey", &m_textureKey);
	ExposeVariable("Size", &m_size);
	ExposeVariable("FillAmount", &m_fillAmount);
	ExposeVariable("FlipX", &m_RenderCommand.bitmap.flipX);
	ExposeVariable("FlipY", &m_RenderCommand.bitmap.flipY);
	ExposeVariable("SrcRect", &m_RenderCommand.srcRect);
}

void UIImageComponent::SetAsSprite(const Sprite& sprite)
{
	m_RenderCommand.bitmap.pTexture = sprite.pTexture;
	m_RenderCommand.srcRect = sprite.srcRect;
	m_RenderCommand.pivot = sprite.pivot;
}

void UIImageComponent::SetTextureKey(const std::wstring& textureKey)
{
	m_textureKey = textureKey;
	m_RenderCommand.bitmap.pTexture = ResourceManager::GetInstance()->GetTexture(textureKey);
}

void UIImageComponent::SetAsBitmap(ID2D1Bitmap* pBitmap, D2D1_RECT_F srcRect)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.bitmap.pTexture = pBitmap;
	m_RenderCommand.srcRect = srcRect;
	m_textureKey = L"";
}

void UIImageComponent::SetAsBitmap(const std::wstring& textureKey, D2D1_RECT_F srcRect)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.srcRect = srcRect;
	SetTextureKey(textureKey);
}

void UIImageComponent::SetNativeSize()
{
	if (!m_RenderCommand.bitmap.pTexture && !m_textureKey.empty())
	{
		m_RenderCommand.bitmap.pTexture = ResourceManager::GetInstance()->GetTexture(m_textureKey);
	}

	if (m_RenderCommand.bitmap.pTexture)
	{
		D2D1_SIZE_F size = m_RenderCommand.bitmap.pTexture->GetSize();
		m_size = { size.width, size.height };
		m_RenderCommand.srcRect = D2D1::RectF(0.0f, 0.0f, size.width, size.height);
	}
}

void UIImageComponent::PostDeserialize(Scene* pScene)
{
	RenderComponent::PostDeserialize(pScene);

	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.isUI = true;
	if (!m_textureKey.empty())
	{
		SetTextureKey(m_textureKey);
	}
}