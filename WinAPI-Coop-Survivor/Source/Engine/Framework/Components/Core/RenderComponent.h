#pragma once
#include "Engine/Framework/Base/Component.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Sprite.h"

class RenderComponent : public Component
{
public:
	CLONEABLE_COMPONENT(RenderComponent)

	RenderComponent(GameObject* owner, TransformComponent* transform);
	virtual ~RenderComponent() override = default;

	virtual const RenderCommand& GetRenderCommand() { return m_RenderCommand; }
	void SetZOrder(int zOrder) { m_RenderCommand.zOrder = zOrder; }
	void SetOpacity(float opacity) { m_RenderCommand.bitmap.opacity = opacity; }
	void SetColor(const D2D1::ColorF& color) { m_RenderCommand.color = color; }
	void SetPivot(D2D1_POINT_2F pivot) { m_RenderCommand.pivot = pivot; }
	void SetPivot(float x, float y) { m_RenderCommand.pivot = { x, y }; }

	virtual std::string_view GetComponentType() const override { return EngineKey::Component::Render; }

protected:
	RenderCommand m_RenderCommand;
};