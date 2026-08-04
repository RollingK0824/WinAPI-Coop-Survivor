#pragma once
#include "Engine/Framework/Base/Component.h"

class UITextComponent : public Component
{
public:
	CLONEABLE_COMPONENT(UITextComponent)

	UITextComponent(GameObject* owner, TransformComponent* transform);

	virtual ~UITextComponent() override = default;

	void SetText(const std::wstring& text) { m_text = text; }
	void SetPosition(Vector2 pos) { m_position = pos; }
	void SetFontSize(float size) { m_fontSize = size; }
	void SetColor(D2D1::ColorF color) { m_color = color; }

	void RenderUI();

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::UITextComponent;
	}

private:
	std::wstring m_text = L"";
	Vector2 m_position = { 10.0f, 10.0f };
	float m_fontSize = 14.0f;
	D2D1::ColorF m_color = D2D1::ColorF(D2D1::ColorF::Yellow); 
};