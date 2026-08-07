#pragma once
#include "Engine/Framework/Components/Core/RenderComponent.h"
#include "Engine/Renderer/Sprite.h"

class UIImageComponent : public RenderComponent
{
public:
	CLONEABLE_COMPONENT(UIImageComponent)

	UIImageComponent(GameObject* owner, TransformComponent* transform);
	virtual ~UIImageComponent() override = default;

	void SetAsSprite(const Sprite& sprite);
	void SetSpriteKey(const std::wstring& spriteKey);
	const std::wstring& GetSpriteKey() const { return m_spriteKey; }

	void SetTextureKey(const std::wstring& textureKey) { SetSpriteKey(textureKey); }
	void SetAsBitmap(ID2D1Bitmap* pBitmap, D2D1_RECT_F srcRect);
	void SetAsBitmap(const std::wstring& textureKey, D2D1_RECT_F srcRect);

	void SetNativeSize();
	void SetFlip(bool flipX, bool flipY) { m_RenderCommand.bitmap.flipX = flipX; m_RenderCommand.bitmap.flipY = flipY; }
	void SetScale(float scaleX, float scaleY) { m_RenderCommand.scaleX = scaleX; m_RenderCommand.scaleY = scaleY; }

	void SetFillAmount(float fill) { m_fillAmount = (fill < 0.0f) ? 0.0f : (fill > 1.0f) ? 1.0f : fill; }
	void SetSize(Vector2 size) { m_size = size; }

	Vector2 GetSize() const { return m_size; }
	float GetFillAmount() const { return m_fillAmount; }
	const std::wstring& GetTextureKey() const { return m_spriteKey; }

	virtual const RenderCommand& GetRenderCommand() override
	{
		m_RenderCommand.bitmap.size = m_size;
		return m_RenderCommand;
	}

	virtual void PostDeserialize(Scene* pScene) override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::UIImageComponent;
	}

protected:
	std::wstring m_spriteKey = L"";
	Vector2 m_size = { 100.0f, 30.0f };
	float m_fillAmount = 1.0f;
};