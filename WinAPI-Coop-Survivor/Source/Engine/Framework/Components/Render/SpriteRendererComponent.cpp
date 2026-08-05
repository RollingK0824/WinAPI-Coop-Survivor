#include "Engine/Core/pch.h"
#include "SpriteRendererComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/ResourceManager.h"

static ComponentRegistrar<SpriteRendererComponent> registrar(EngineKey::Component::SpriteRenderer.data());

SpriteRendererComponent::SpriteRendererComponent(GameObject* owner, TransformComponent* transform) : RenderComponent(owner, transform)
{
	m_RenderCommand.type = RenderType::BITMAP;

	ExposeTexture("Texture Key", &m_textureKey);
	ExposeVariable("Flip X", &m_RenderCommand.bitmap.flipX);
	ExposeVariable("Flip Y", &m_RenderCommand.bitmap.flipY);
	ExposeVariable("Src Left", &m_RenderCommand.srcRect.left);
	ExposeVariable("Src Top", &m_RenderCommand.srcRect.top);
	ExposeVariable("Src Right", &m_RenderCommand.srcRect.right);
	ExposeVariable("Src Bottom", &m_RenderCommand.srcRect.bottom);
}

void SpriteRendererComponent::SetAsSprite(const Sprite& sprite)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.bitmap.pTexture = sprite.pTexture;
	m_RenderCommand.srcRect = sprite.srcRect;
	m_RenderCommand.pivot = sprite.pivot;
}

void SpriteRendererComponent::SetTextureKey(const std::wstring& textureKey)
{
	m_textureKey = textureKey;
	m_RenderCommand.bitmap.pTexture = ResourceManager::GetInstance()->GetTexture(textureKey);
}

void SpriteRendererComponent::SetAsBitmap(ID2D1Bitmap* pBitmap, D2D1_RECT_F srcRect)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.bitmap.pTexture = pBitmap;
	m_RenderCommand.srcRect = srcRect;
	m_textureKey = L"";
}

void SpriteRendererComponent::SetAsBitmap(const std::wstring& textureKey, D2D1_RECT_F srcRect)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.srcRect = srcRect;
	SetTextureKey(textureKey);
}

void SpriteRendererComponent::SetNativeSize()
{
	if (!m_RenderCommand.bitmap.pTexture && !m_textureKey.empty())
	{
		m_RenderCommand.bitmap.pTexture = ResourceManager::GetInstance()->GetTexture(m_textureKey);
	}
	if (m_RenderCommand.bitmap.pTexture)
	{
		D2D1_SIZE_F size = m_RenderCommand.bitmap.pTexture->GetSize();
		m_RenderCommand.srcRect = D2D1::RectF(0.0f, 0.0f, size.width, size.height);
	}
}

void SpriteRendererComponent::Serialize(json& outJson) const
{
	RenderComponent::Serialize(outJson);
	std::string strKey(m_textureKey.begin(), m_textureKey.end());
	outJson[EngineKey::Property::TextureKey.data()] = strKey;
	outJson["FlipX"] = m_RenderCommand.bitmap.flipX;
	outJson["FlipY"] = m_RenderCommand.bitmap.flipY;
	outJson[EngineKey::Property::SrcRect.data()] =
	{
		{"left", m_RenderCommand.srcRect.left}, {"top", m_RenderCommand.srcRect.top},
		{"right", m_RenderCommand.srcRect.right}, {"bottom", m_RenderCommand.srcRect.bottom}
	};
}

void SpriteRendererComponent::Deserialize(const json& inJson)
{
	RenderComponent::Deserialize(inJson);
	D2D1_RECT_F srcRect = { 0.0f, 0.0f, 0.0f, 0.0f };
	if (inJson.contains(EngineKey::Property::SrcRect.data()))
	{
		const auto& rect = inJson[EngineKey::Property::SrcRect.data()];
		srcRect = D2D1::RectF(
			rect["left"].get<float>(), rect["top"].get<float>(),
			rect["right"].get<float>(), rect["bottom"].get<float>()
		);
	}
	if (inJson.contains(EngineKey::Property::TextureKey.data()))
	{
		std::string strKey = inJson[EngineKey::Property::TextureKey.data()].get<std::string>();
		std::wstring wstrKey(strKey.begin(), strKey.end());
		if (!wstrKey.empty())
		{
			SetAsBitmap(wstrKey, srcRect);
		}
	}
	if (inJson.contains("FlipX")) m_RenderCommand.bitmap.flipX = inJson["FlipX"].get<bool>();
	if (inJson.contains("FlipY")) m_RenderCommand.bitmap.flipY = inJson["FlipY"].get<bool>();
}
