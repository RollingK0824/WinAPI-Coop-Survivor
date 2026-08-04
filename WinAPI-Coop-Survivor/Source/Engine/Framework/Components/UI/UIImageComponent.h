#pragma once
#include "Engine/Framework/Components/UI/UIComponent.h"
#include "Engine/Renderer/RenderCommand.h"

class UIImageComponent : public UIComponent
{
public:
	CLONEABLE_COMPONENT(UIImageComponent)

		UIImageComponent(GameObject* owner, TransformComponent* transform);
	virtual ~UIImageComponent() override = default;

	virtual void Serialize(json& outJson) const override;
	virtual void Deserialize(const json& inJson) override;

	void SetTextureKey(const std::wstring& textureKey);
	void SetPosition(Vector2 pos) { m_RenderCommand.position = pos; }
	void SetSize(Vector2 size) { m_size = size; }
	void SetFillAmount(float fill) { m_fillAmount = (fill < 0.0f) ? 0.0f : (fill > 1.0f) ? 1.0f : fill; }
	void SetOpacity(float opacity) { m_RenderCommand.bitmap.opacity = opacity; }
	void SetZOrder(int zOrder) { m_RenderCommand.zOrder = zOrder; }

	Vector2 GetPosition() const { return m_RenderCommand.position; }
	Vector2 GetSize() const { return m_size; }
	float GetFillAmount() const { return m_fillAmount; }

	const RenderCommand& GetRenderCommand() const { return m_RenderCommand; }

	void RenderUI();

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::UIImageComponent;
	}

private:
	RenderCommand m_RenderCommand; 
	std::wstring m_textureKey = L"";

	Vector2 m_size = { 100.0f, 30.0f };
	float m_fillAmount = 1.0f;
};