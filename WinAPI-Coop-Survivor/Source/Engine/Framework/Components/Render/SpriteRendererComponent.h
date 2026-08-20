#pragma once
#include "Engine/Framework/Components/Core/RenderComponent.h"
#include "Engine/Renderer/Sprite.h"

class SpriteRendererComponent : public RenderComponent
{
public:
	CLONEABLE_COMPONENT(SpriteRendererComponent)

	SpriteRendererComponent(GameObject* owner, TransformComponent* transform);
	virtual ~SpriteRendererComponent() override = default;

	virtual void Awake() override;

	void SetAsSprite(const Sprite& sprite);

	void SetSpriteKey(const std::wstring& spriteKey);
	const std::wstring& GetSpriteKey() const { return m_spriteKey; }

	void SetTextureKey(const std::wstring& textureKey) { SetSpriteKey(textureKey); }
	const std::wstring& GetTextureKey() const { return m_spriteKey; }

	void SetAsBitmap(ID2D1Bitmap* pBitmap, D2D1_RECT_F srcRect);
	void SetAsBitmap(const std::wstring& textureKey, D2D1_RECT_F srcRect);

	void SetNativeSize();
	void SetSize(Vector2 size);
	Vector2 GetSize() const { return m_size; }

	Vector2 GetSpriteSize() const;

	void SetFlip(bool flipX, bool flipY) { m_RenderCommand.bitmap.flipX = flipX; m_RenderCommand.bitmap.flipY = flipY; }
	void SetFlipX(bool flipX) { m_RenderCommand.bitmap.flipX = flipX; }
	void SetFlipY(bool flipY) { m_RenderCommand.bitmap.flipY = flipY; }
	bool GetFlipX() const { return m_RenderCommand.bitmap.flipX; }
	bool GetFlipY() const { return m_RenderCommand.bitmap.flipY; }
	void SetScale(float scaleX, float scaleY) { m_RenderCommand.scaleX = scaleX; m_RenderCommand.scaleY = scaleY; }

	void SetBorder(const D2D1_RECT_F& border) { m_RenderCommand.bitmap.sprite.border = border; }
	void SetBorder(float left, float top, float right, float bottom) { m_RenderCommand.bitmap.sprite.border = D2D1::RectF(left, top, right, bottom); }
	const D2D1_RECT_F& GetBorder() const { return m_RenderCommand.bitmap.sprite.border; }

	virtual const RenderCommand& GetRenderCommand() override
	{
		m_RenderCommand.bitmap.size = m_size;
		return m_RenderCommand;
	}

	virtual void PostDeserialize(Scene* pScene) override;

	virtual std::string_view GetComponentType() const override { return EngineKey::Component::SpriteRenderer; }

protected:
	std::wstring m_spriteKey = L"";
	Vector2 m_size = { 0.0f, 0.0f };
};