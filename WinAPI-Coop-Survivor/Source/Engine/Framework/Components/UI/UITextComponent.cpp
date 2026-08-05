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
	ExposeVariable("FontSize", &m_RenderCommand.text.fontSize);
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

void UITextComponent::PostDeserialize(Scene* pScene)
{
	RenderComponent::PostDeserialize(pScene);

	m_RenderCommand.type = RenderType::TEXT;
	m_RenderCommand.isUI = true;
	m_RenderCommand.text.pText = m_text;
}