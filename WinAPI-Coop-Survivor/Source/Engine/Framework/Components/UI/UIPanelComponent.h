#pragma once
#include "Engine/Framework/Base/Component.h"
#include "Engine/Renderer/RenderCommand.h"

class UIPanelComponent : public Component
{
public:
	CLONEABLE_COMPONENT(UIPanelComponent)

		UIPanelComponent(GameObject* owner, TransformComponent* transform);
	virtual ~UIPanelComponent() override = default;

	void AddChildUI(GameObject* pChildUI);

	void SetTextureKey(const std::wstring& textureKey) { m_textureKey = textureKey; }
	void SetSize(Vector2 size) { m_size = size; }
	void SetColor(const D2D1::ColorF& color) { m_color = color; }
	void SetOpacity(float opacity) { m_RenderCommand.bitmap.opacity = opacity; }
	void SetZOrder(int zOrder) { m_RenderCommand.zOrder = zOrder; }
	void SetRenderBackground(bool bRender) { m_bRenderBackground = bRender; }

	Vector2 GetSize() const { return m_size; }
	D2D1_COLOR_F GetColor() const { return m_color; }
	bool IsRenderBackground() const { return m_bRenderBackground; }

	void RenderUI();

	virtual void OnEnable() override;
	virtual void OnDisable() override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::UIPanelComponent;
	}

private:
	std::vector<GameObject*> m_vChildUIObjects;

	RenderCommand m_RenderCommand;
	std::wstring m_textureKey = L"";

	Vector2 m_size = { 200.0f, 150.0f };
	D2D1_COLOR_F m_color = D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.6f);
	bool m_bRenderBackground = true;
};