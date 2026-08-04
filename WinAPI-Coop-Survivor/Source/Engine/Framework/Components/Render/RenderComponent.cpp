#include "Engine/Core/pch.h"
#include "RenderComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/ResourceManager.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"

static ComponentRegistrar<RenderComponent> registrar(EngineKey::Component::Render.data());

RenderComponent::RenderComponent(GameObject* owner, TransformComponent* transform) : Component(owner, transform) 
{
	ExposeVariable("Z-Order", &m_RenderCommand.zOrder);
	ExposeVariable("Opacity", &m_RenderCommand.bitmap.opacity);
	ExposeVariable("Flip X", &m_RenderCommand.bitmap.flipX);
	ExposeVariable("Flip Y", &m_RenderCommand.bitmap.flipY);

	ExposeVariable("Src Left", &m_RenderCommand.srcRect.left);
	ExposeVariable("Src Top", &m_RenderCommand.srcRect.top);
	ExposeVariable("Src Right", &m_RenderCommand.srcRect.right);
	ExposeVariable("Src Bottom", &m_RenderCommand.srcRect.bottom);

	ExposeTexture("Texture Key", &m_textureKey);
	ExposeVariable("Color", &m_RenderCommand.color);
}
void RenderComponent::SetNativeSize()
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

void RenderComponent::SetTextureKey(const std::wstring& textureKey)
{
	m_textureKey = textureKey;
	m_RenderCommand.bitmap.pTexture = ResourceManager::GetInstance()->GetTexture(textureKey);

	SetNativeSize();
}

void RenderComponent::SetAsSprite(const Sprite& sprite)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.bitmap.pTexture = sprite.pTexture;
	m_RenderCommand.srcRect = sprite.srcRect;
	
	m_RenderCommand.bitmap.offset = sprite.offset;
	m_RenderCommand.bitmap.originalWidth = sprite.originalWidth;
	m_RenderCommand.bitmap.originalHeight = sprite.originalHeight;
	m_RenderCommand.pivot = sprite.pivot;
}

void RenderComponent::SetAsBitmap(ID2D1Bitmap* pBitmap, D2D1_RECT_F srcRect)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.bitmap.pTexture = pBitmap;
	m_RenderCommand.srcRect = srcRect;

	m_textureKey = L"";
}

void RenderComponent::SetAsBitmap(const std::wstring& textureKey, D2D1_RECT_F srcRect)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_textureKey = textureKey;
	m_RenderCommand.srcRect = srcRect;

	m_RenderCommand.bitmap.pTexture = ResourceManager::GetInstance()->GetTexture(textureKey);
}

void RenderComponent::Serialize(json& outJson) const
{
	Component::Serialize(outJson);

	outJson[EngineKey::Property::RenderType.data()] = static_cast<int>(m_RenderCommand.type);
	outJson[EngineKey::Property::ZOrder.data()] = m_RenderCommand.zOrder;

	std::string strKey(m_textureKey.begin(), m_textureKey.end());
	outJson[EngineKey::Property::TextureKey.data()] = strKey;

	outJson[EngineKey::Property::SrcRect.data()] =
	{
		{"left",m_RenderCommand.srcRect.left},{"top",m_RenderCommand.srcRect.top},
		{"right",m_RenderCommand.srcRect.right},{"bottom",m_RenderCommand.srcRect.bottom}
	};
}

void RenderComponent::Deserialize(const json& inJson)
{
	Component::Deserialize(inJson);

	if (inJson.contains(EngineKey::Property::RenderType))
		m_RenderCommand.type = static_cast<RenderType>(inJson[EngineKey::Property::RenderType.data()].get<int>());

	if (inJson.contains(EngineKey::Property::ZOrder))
		m_RenderCommand.zOrder = inJson[EngineKey::Property::ZOrder].get<int>();

	D2D1_RECT_F srcRect = { 0.0f,0.0f,0.0f,0.0f };
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

		if (!wstrKey.empty() && m_RenderCommand.type == RenderType::BITMAP)
		{
			this->SetAsBitmap(wstrKey, srcRect);
		}
	}
}
