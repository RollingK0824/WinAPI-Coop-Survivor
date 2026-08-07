#pragma once
#include "Engine/Framework/Components/Physics/ColliderComponent.h"

class BoxCollider : public ColliderComponent
{
public:
    CLONEABLE_COMPONENT(BoxCollider)

    BoxCollider(GameObject* owner, TransformComponent* transform);
	virtual ~BoxCollider() override = default;

	virtual void Awake() override;
	virtual void PostDeserialize(Scene* pScene) override;

	void SetSize(float width, float height)
	{
		m_size.x = width;
		m_size.y = height;

		RebuildShape();
	}

    virtual std::string_view GetComponentType() const override
    {
        return EngineKey::Component::BoxCollider;
    }

    virtual void DrawDebug() override;

protected:
	virtual b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef* shapeDef) override
	{
		if (m_size.x <= 0.0f || m_size.y <= 0.0f) return b2_nullShapeId;

		b2Polygon box = b2MakeOffsetBox(
			PixelToMeter(m_size.x * 0.5f),
			PixelToMeter(m_size.y * 0.5f),
			b2Vec2{ PixelToMeter(m_offset.x), PixelToMeter(m_offset.y) },
			b2Rot_identity
		);
		return b2CreatePolygonShape(bodyId, shapeDef, &box);
	}

private:
	Vector2 m_size;
};
