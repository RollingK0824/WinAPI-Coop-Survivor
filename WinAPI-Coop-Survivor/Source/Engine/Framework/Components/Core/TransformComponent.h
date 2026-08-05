#pragma once
#include "Engine/Framework/Base/Component.h"

class TransformComponent : public Component
{
public:
	CLONEABLE_COMPONENT(TransformComponent)

	TransformComponent(GameObject* owner);
	virtual ~TransformComponent() override = default;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::Trnasform;
	}

	void SetSiblingIndex(int index);
	int GetSiblingIndex() const;
	void SetAsFirstSibling();
	void SetAsLastSibling();

	const Vector2& GetPosition() const { return m_Position; }
	void SetPosition(float x, float y) { m_Position.x = x; m_Position.y = y; }
	void SetPosition(Vector2 position) { m_Position = position; }

	const Rotation& GetRotation() const { return m_Rotation; }
	void SetRotation(float angle) { m_Rotation.angle = angle; }
	void SetRotation(Rotation rotation) { m_Rotation = rotation; }

	const Vector2& GetScale() const { return m_Scale; }
	void SetScale(float scaleX, float scaleY) { m_Scale.x = scaleX; m_Scale.y = scaleY; }
	void SetScale(Vector2 scale) { m_Scale = scale; }

private:
	Vector2 m_Position = { 0.0f, 0.0f };
	Rotation m_Rotation = { 0.0f };
	Vector2 m_Scale = { 1.0f, 1.0f };
};
