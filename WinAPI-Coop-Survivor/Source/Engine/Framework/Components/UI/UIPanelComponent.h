// Source/Engine/Framework/Components/UI/UIPanelComponent.h
#pragma once
#include "Engine/Framework/Components/Core/RenderComponent.h"

class UIPanelComponent : public RenderComponent
{
public:
	CLONEABLE_COMPONENT(UIPanelComponent)

	UIPanelComponent(GameObject* owner, TransformComponent* transform);
	virtual ~UIPanelComponent() override = default;

	void SetTextureKey(const std::wstring& textureKey);
	void SetSize(Vector2 size) { m_size = size; }
	void SetRenderBackground(bool bRender) { m_bRenderBackground = bRender; }

	Vector2 GetSize() const { return m_size; }
	const std::wstring& GetTextureKey() const { return m_textureKey; }
	bool IsRenderBackground() const { return m_bRenderBackground; }

	virtual void PostDeserialize(Scene* pScene) override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::UIPanelComponent;
	}

protected:
	std::wstring m_textureKey = L"";
	Vector2 m_size = { 200.0f, 150.0f };
	bool m_bRenderBackground = true;
};