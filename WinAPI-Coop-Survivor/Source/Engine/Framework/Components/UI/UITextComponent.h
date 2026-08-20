#pragma once
#include "Engine/Framework/Components/Core/RenderComponent.h"

class UITextComponent : public RenderComponent
{
public:
	CLONEABLE_COMPONENT(UITextComponent)

	UITextComponent(GameObject* owner, TransformComponent* transform);
	virtual ~UITextComponent() override = default;

	virtual const RenderCommand& GetRenderCommand() override;

	void SetText(const std::wstring& text);
	void SetFontSize(float size);
	void SetColor(const D2D1::ColorF& color);
	void SetColor(const D2D1_COLOR_F& color) { m_RenderCommand.color = color; }
	void SetAlignment(ETextAlignment alignment);
	void SetParagraphAlignment(EParagraphAlignment paragraphAlignment);
	void SetSize(Vector2 size);
	void SetSize(float w, float h);
	void SetPivot(D2D1_POINT_2F pivot);
	void SetPivot(float px, float py);

	const std::wstring& GetText() const { return m_text; }
	float GetFontSize() const { return m_RenderCommand.text.fontSize; }
	D2D1_COLOR_F GetColor() const { return m_RenderCommand.color; }
	ETextAlignment GetAlignment() const { return m_alignment; }
	EParagraphAlignment GetParagraphAlignment() const { return m_paragraphAlignment; }
	Vector2 GetSize() const { return m_size; }
	D2D1_POINT_2F GetPivot() const { return m_pivot; }

	virtual void PostDeserialize(Scene* pScene) override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::UITextComponent;
	}

private:
	std::wstring m_text = L"New Text";
	ETextAlignment m_alignment = ETextAlignment::Center;
	EParagraphAlignment m_paragraphAlignment = EParagraphAlignment::Center;
	Vector2 m_size = { 300.0f, 100.0f };
	D2D1_POINT_2F m_pivot = { 0.5f, 0.5f };
};