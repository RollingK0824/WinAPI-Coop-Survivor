// Source/Engine/Framework/Components/UI/UITextComponent.cpp
#include "Engine/Core/pch.h"
#include "UITextComponent.h"
#include "Engine/Core/ComponentRegister.h"

static ComponentRegistrar<UITextComponent> registrar(EngineKey::Component::UITextComponent.data());

UITextComponent::UITextComponent(GameObject* owner, TransformComponent* transform)
	: RenderComponent(owner, transform)
{
	m_RenderCommand.type = RenderType::TEXT;
	m_RenderCommand.isUI = true;
	m_RenderCommand.zOrder = 9999;
	m_RenderCommand.text.pText = m_text;
	m_RenderCommand.text.fontSize = 14.0f;
	m_RenderCommand.color = D2D1::ColorF(D2D1::ColorF::Yellow);

	ExposeVariable("Text", &m_text);
	ExposeVariable("Font Size", &m_RenderCommand.text.fontSize);
}

void UITextComponent::SetText(const std::wstring& text)
{
	m_text = text;
	m_RenderCommand.text.pText = m_text;
}

void UITextComponent::SetFontSize(float size)
{
	m_RenderCommand.text.fontSize = size;
}

void UITextComponent::SetColor(const D2D1::ColorF& color)
{
	m_RenderCommand.color = color;
}

void UITextComponent::Serialize(json& outJson) const
{
	RenderComponent::Serialize(outJson);

	std::string strText(m_text.begin(), m_text.end());
	outJson["Text"] = strText;
	outJson["FontSize"] = m_RenderCommand.text.fontSize;
}

void UITextComponent::Deserialize(const json& inJson)
{
	RenderComponent::Deserialize(inJson);

	if (inJson.contains("Text"))
	{
		std::string strText = inJson["Text"].get<std::string>();
		SetText(std::wstring(strText.begin(), strText.end()));
	}
	if (inJson.contains("FontSize"))
	{
		SetFontSize(inJson["FontSize"].get<float>());
	}
}