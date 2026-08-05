#pragma once
#include "Engine/Framework/Components/Core/RenderComponent.h"
#include "Engine/Renderer/Sprite.h"

class SpriteRendererComponent : public RenderComponent
{
public:
	CLONEABLE_COMPONENT(SpriteRendererComponent)

	SpriteRendererComponent(GameObject* owner, TransformComponent* transform);
	virtual ~SpriteRendererComponent() override = default;

	void SetAsSprite(const Sprite& sprite);

	void SetTextureKey(const std::wstring& textureKey);
	const std::wstring& GetTextureKey() const { return m_textureKey; }

	void SetAsBitmap(ID2D1Bitmap* pBitmap, D2D1_RECT_F srcRect);
	void SetAsBitmap(const std::wstring& textureKey, D2D1_RECT_F srcRect);

	void SetNativeSize();

	void SetFlip(bool flipX, bool flipY) { m_RenderCommand.bitmap.flipX = flipX; m_RenderCommand.bitmap.flipY = flipY; }
	void SetScale(float scaleX, float scaleY) { m_RenderCommand.scaleX = scaleX; m_RenderCommand.scaleY = scaleY; }

	virtual void PostDeserialize(Scene* pScene) override;

	virtual std::string_view GetComponentType() const override { return EngineKey::Component::SpriteRenderer; }

protected:
	std::wstring m_textureKey = L"";
};