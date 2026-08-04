#pragma once
#include "Engine/Framework/Components/Physics/ColliderComponent.h"

class CircleCollider : public ColliderComponent
{
public:
	CircleCollider() = default;
	virtual ~CircleCollider() = default;

    virtual void Serialize(json& outJson) const override
    {
        ColliderComponent::Serialize(outJson);
        outJson["Radius"] = m_Radius;
    }

    virtual void Deserialize(const json& inJson) override
    {
        ColliderComponent::Deserialize(inJson);
        if (inJson.contains("Radius")) m_Radius = inJson["Radius"].get<float>();
        RebuildShape();
    }

	void SetRadius(float radius) 
	{ 
		m_Radius = radius; 
		RebuildShape();
	}

protected:
	virtual b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef* shapeDef)override
	{
		if (m_Radius <= 0.0f)return b2_nullShapeId;

		b2Circle circle;
		circle.center = b2Vec2{ 0.0f,0.0f };
		circle.radius = PixelToMeter(m_Radius);
		return b2CreateCircleShape(bodyId, shapeDef, &circle);
	}

private:
	float m_Radius = 0.0f;
};