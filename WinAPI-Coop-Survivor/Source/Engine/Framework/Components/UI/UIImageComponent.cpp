#include "Engine/Core/pch.h"
#include "UIImageComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/ResourceManager.h"
#include "Engine/Renderer/RenderSystem.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"

static ComponentRegistrar<UIImageComponent> registrar(EngineKey::Component::UIImageComponent.data());

UIImageComponent::UIImageComponent(GameObject* owner, TransformComponent* transform)
	: UIComponent(owner, transform)
{
	m_RenderCommand.type = RenderType::BITMAP;
	m_RenderCommand.isUI = true; // Screen Space ∞Ì¡§
	m_RenderCommand.zOrder = 90000;

	ExposeTexture("Texture Key", &m_textureKey);
	ExposeVariable("Position", &m_RenderCommand.position);
	ExposeVariable("Size", &m_size);
	ExposeVariable("Fill Amount", &m_fillAmount);
	ExposeVariable("Opacity", &m_RenderCommand.bitmap.opacity);
	ExposeVariable("Z-Order", &m_RenderCommand.zOrder);
}

void UIImageComponent::Serialize(json& outJson) const
{
	UIComponent::Serialize(outJson);

	std::string strKey(m_textureKey.begin(), m_textureKey.end());
	outJson["TextureKey"] = strKey;
	outJson["Size"] = { {"x", m_size.x}, {"y", m_size.y} };
	outJson["Position"] = { {"x", m_RenderCommand.position.x}, {"y", m_RenderCommand.position.y} };
	outJson["FillAmount"] = m_fillAmount;
	outJson["Opacity"] = m_RenderCommand.bitmap.opacity;
	outJson["ZOrder"] = m_RenderCommand.zOrder;
}

void UIImageComponent::Deserialize(const json& inJson)
{
	UIComponent::Deserialize(inJson);

	if (inJson.contains("TextureKey"))
	{
		std::string strKey = inJson["TextureKey"].get<std::string>();
		SetTextureKey(std::wstring(strKey.begin(), strKey.end()));
	}

	if (inJson.contains("Size"))
	{
		std::string strKey = inJson["TextureKey"].get<std::string>();
		SetTextureKey(std::wstring(strKey.begin(), strKey.end()));
	}
}

void UIImageComponent::SetTextureKey(const std::wstring& textureKey)
{
	m_textureKey = textureKey;
	m_RenderCommand.bitmap.pTexture = ResourceManager::GetInstance()->GetTexture(textureKey);
}

void UIImageComponent::RenderUI()
{
	if (!m_bIsEnabled || !gameObject.IsActive() || !m_RenderCommand.bitmap.pTexture) return;

	D2D1_SIZE_F texSize = m_RenderCommand.bitmap.pTexture->GetSize();

	RenderCommand cmd = m_RenderCommand;
	cmd.position = transform.GetPosition() + m_RenderCommand.position;
	cmd.scaleX = (m_size.x * m_fillAmount) / texSize.width;
	cmd.scaleY = m_size.y / texSize.height;
	cmd.srcRect = D2D1::RectF(0.0f, 0.0f, texSize.width * m_fillAmount, texSize.height);

	RenderSystem::GetInstance()->SubmitCommand(cmd);
}