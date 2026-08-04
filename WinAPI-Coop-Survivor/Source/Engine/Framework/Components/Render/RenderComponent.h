#pragma once
#include "Engine/Framework/Base/Component.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Sprite.h"

class RenderComponent : public Component
{
public:
	CLONEABLE_COMPONENT(RenderComponent)

	RenderComponent(GameObject* owner, TransformComponent* transform);
	virtual ~RenderComponent()override = default;

	void SetAsCircle(float radius, D2D1::ColorF color, bool isFilled = true)
	{
		m_RenderCommand.type = RenderType::DEBUG_CIRCLE;
		m_RenderCommand.srcRect.left = radius; // 반지름 규격 보관
		m_RenderCommand.color = color;
		m_RenderCommand.shape.isFilled = isFilled;
	}

	void SetAsRect(float width, float height, D2D1::ColorF color, bool isFilled = true)
	{
		m_RenderCommand.type = RenderType::DEBUG_RECT;
		m_RenderCommand.srcRect = D2D1::RectF(0.0f, 0.0f, width, height);
		m_RenderCommand.color = color;
		m_RenderCommand.shape.isFilled = isFilled;
	}
	void SetNativeSize();

	void SetAsSprite(const Sprite& sprite);

	void SetTextureKey(const std::wstring& textureKey);
	void SetAsBitmap(ID2D1Bitmap* pBitmap, D2D1_RECT_F srcRect);
	void SetAsBitmap(const std::wstring& textureKey, D2D1_RECT_F srcRect);

	void SetZOrder(int zOrder) { m_RenderCommand.zOrder = zOrder; }
	void SetOpacity(float opacity) { m_RenderCommand.bitmap.opacity = opacity; }
	void SetFlip(bool flipX, bool flipY) { m_RenderCommand.bitmap.flipX = flipX; m_RenderCommand.bitmap.flipY = flipY; }
	void SetScale(float scaleX, float scaleY) { m_RenderCommand.scaleX = scaleX; m_RenderCommand.scaleY = scaleY; }


	// RenderCommand Getter
	const RenderCommand& GetRenderCommand() const { return m_RenderCommand; }
	const std::wstring& GetTextureKey() const { return m_textureKey; }

	std::string_view GetComponentType() const { return EngineKey::Component::Render; }
	void Serialize(json& outJson)const override;
	void Deserialize(const json& inJson)override;

private:
	RenderCommand m_RenderCommand;
	std::wstring m_textureKey = L"";
};