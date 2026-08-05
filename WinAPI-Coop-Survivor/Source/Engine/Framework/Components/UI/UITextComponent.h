// Source/Engine/Framework/Components/UI/UITextComponent.h
#pragma once
#include "Engine/Framework/Components/Core/RenderComponent.h"

class UITextComponent : public RenderComponent
{
public:
	CLONEABLE_COMPONENT(UITextComponent)

	UITextComponent(GameObject* owner, TransformComponent* transform);
	virtual ~UITextComponent() override = default;

	void SetText(const std::wstring& text);
	void SetFontSize(float size);
	void SetColor(const D2D1::ColorF& color);

	const std::wstring& GetText() const { return m_text; }
	float GetFontSize() const { return m_RenderCommand.text.fontSize; }
	D2D1_COLOR_F GetColor() const { return m_RenderCommand.color; }

	virtual void PostDeserialize(Scene* pScene) override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::UITextComponent;
	}

private:
	std::wstring m_text = L"New Text";
};