#pragma once
#include "Engine/Framework/Components/Physics/ColliderComponent.h"

class BoxCollider : public ColliderComponent
{
public:
    CLONEABLE_COMPONENT(BoxCollider)

    BoxCollider(GameObject* owner, TransformComponent* transform);
	virtual ~BoxCollider() override = default;

	void SetSize(float width, float height)
	{
		m_HalfWidth = width * 0.5f;
		m_HalfHeight = height * 0.5f;

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
		if (m_HalfWidth <= 0.0f || m_HalfHeight <= 0.0f) return b2_nullShapeId;

		b2Polygon box = b2MakeBox(PixelToMeter(m_HalfWidth), PixelToMeter(m_HalfHeight));
		return b2CreatePolygonShape(bodyId, shapeDef, &box);
	}

private:
	float m_HalfWidth = 0.0f;
	float m_HalfHeight = 0.0f;
};
