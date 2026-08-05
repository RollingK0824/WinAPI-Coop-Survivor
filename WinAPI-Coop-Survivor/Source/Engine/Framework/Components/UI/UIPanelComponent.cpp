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

	ExposeTexture("Texture Key", &m_textureKey);
	ExposeVariable("Size", &m_size);
	ExposeVariable("Render Background", &m_bRenderBackground);
}

void UIPanelComponent::SetTextureKey(const std::wstring& textureKey)
{
	m_textureKey = textureKey;
	m_RenderCommand.bitmap.pTexture = ResourceManager::GetInstance()->GetTexture(textureKey);
	if (m_RenderCommand.bitmap.pTexture != nullptr)
	{
		m_RenderCommand.type = RenderType::BITMAP;
	}
}

void UIPanelComponent::Serialize(json& outJson) const
{
	RenderComponent::Serialize(outJson);

	std::string strKey(m_textureKey.begin(), m_textureKey.end());
	outJson["TextureKey"] = strKey;
	outJson["Size"] = { {"x", m_size.x}, {"y", m_size.y} };
	outJson["RenderBackground"] = m_bRenderBackground;
}

void UIPanelComponent::Deserialize(const json& inJson)
{
	RenderComponent::Deserialize(inJson);

	if (inJson.contains("TextureKey"))
	{
		std::string strKey = inJson["TextureKey"].get<std::string>();
		SetTextureKey(std::wstring(strKey.begin(), strKey.end()));
	}
	if (inJson.contains("Size"))
	{
		m_size.x = inJson["Size"]["x"].get<float>();
		m_size.y = inJson["Size"]["y"].get<float>();
	}
	if (inJson.contains("RenderBackground"))
	{
		m_bRenderBackground = inJson["RenderBackground"].get<bool>();
	}
}