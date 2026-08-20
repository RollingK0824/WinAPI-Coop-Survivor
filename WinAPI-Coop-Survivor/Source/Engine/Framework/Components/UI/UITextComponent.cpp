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
	m_RenderCommand.text.alignment = m_alignment;
	m_RenderCommand.text.paragraphAlignment = m_paragraphAlignment;
	m_RenderCommand.text.size = m_size;
	m_RenderCommand.pivot = m_pivot;
	m_RenderCommand.color = D2D1::ColorF(D2D1::ColorF::Yellow);

	ExposeVariable("Text", &m_text);
	ExposeVariable("FontSize", &m_RenderCommand.text.fontSize);
	ExposeVariable("Alignment", reinterpret_cast<uint8*>(&m_alignment));
	ExposeVariable("ParagraphAlignment", reinterpret_cast<uint8*>(&m_paragraphAlignment));
	ExposeVariable("Size", &m_size);
	ExposeVariable("Pivot", &m_pivot);
}

const RenderCommand& UITextComponent::GetRenderCommand()
{
	m_RenderCommand.text.pText = m_text;
	m_RenderCommand.text.alignment = m_alignment;
	m_RenderCommand.text.paragraphAlignment = m_paragraphAlignment;
	m_RenderCommand.text.size = m_size;
	m_RenderCommand.pivot = m_pivot;
	return m_RenderCommand;
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

void UITextComponent::SetAlignment(ETextAlignment alignment)
{
	m_alignment = alignment;
	m_RenderCommand.text.alignment = alignment;
}

void UITextComponent::SetParagraphAlignment(EParagraphAlignment paragraphAlignment)
{
	m_paragraphAlignment = paragraphAlignment;
	m_RenderCommand.text.paragraphAlignment = paragraphAlignment;
}

void UITextComponent::SetSize(Vector2 size)
{
	m_size = size;
	m_RenderCommand.text.size = size;
}

void UITextComponent::SetSize(float w, float h)
{
	m_size = { w, h };
	m_RenderCommand.text.size = m_size;
}

void UITextComponent::SetPivot(D2D1_POINT_2F pivot)
{
	m_pivot = pivot;
	m_RenderCommand.pivot = pivot;
}

void UITextComponent::SetPivot(float px, float py)
{
	m_pivot = { px, py };
	m_RenderCommand.pivot = m_pivot;
}

void UITextComponent::PostDeserialize(Scene* pScene)
{
	RenderComponent::PostDeserialize(pScene);

	m_RenderCommand.type = RenderType::TEXT;
	m_RenderCommand.isUI = true;
	m_RenderCommand.text.pText = m_text;
	m_RenderCommand.text.alignment = m_alignment;
	m_RenderCommand.text.paragraphAlignment = m_paragraphAlignment;
	m_RenderCommand.text.size = m_size;
	m_RenderCommand.pivot = m_pivot;
}