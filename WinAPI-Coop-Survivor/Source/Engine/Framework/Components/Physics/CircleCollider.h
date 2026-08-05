#pragma once
#include "Engine/Framework/Components/Physics/ColliderComponent.h"

class CircleCollider : public ColliderComponent
{
public:
	CLONEABLE_COMPONENT(CircleCollider)

	CircleCollider(GameObject* owner, TransformComponent* transform);
	virtual ~CircleCollider() override = default;

	void SetRadius(float radius) 
	{ 
		m_Radius = radius; 
		RebuildShape();
	}

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::CircleCollider;
	}

protected:
	virtual b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef* shapeDef) override
	{
		if (m_Radius <= 0.0f) return b2_nullShapeId;

		b2Circle circle;
		circle.center = b2Vec2{ 0.0f, 0.0f };
		circle.radius = PixelToMeter(m_Radius);
		return b2CreateCircleShape(bodyId, shapeDef, &circle);
	}

private:
	float m_Radius = 0.0f;
};