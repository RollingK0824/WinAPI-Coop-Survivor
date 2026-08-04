#pragma once
#include "Engine/Framework/Base/Component.h"

class ColliderComponent : public Component
{
public:
	ColliderComponent(GameObject* owner, TransformComponent* transform);
	virtual ~ColliderComponent() = default;

	virtual void OnEnable() override;
	virtual void OnDisable() override;

	virtual void Serialize(json& outJson)const override
	{
		Component::Serialize(outJson);
		outJson[EngineKey::Property::Density.data()] = density;
		outJson[EngineKey::Property::Friction.data()] = friction;
		outJson[EngineKey::Property::Restitution.data()] = restitution;
		outJson[EngineKey::Property::IsSensor.data()] = isSensor;
		outJson[EngineKey::Property::BodyType.data()] = static_cast<int>(m_BodyType);
	}

	virtual void Deserialize(const json& inJson)override
	{
		Component::Deserialize(inJson);
		if (inJson.contains(EngineKey::Property::Density.data()))
		{
			density = inJson[EngineKey::Property::Density.data()].get<float>();
		}
		if (inJson.contains(EngineKey::Property::Friction.data()))
		{
			friction = inJson[EngineKey::Property::Friction.data()].get<float>();
		}
		if (inJson.contains(EngineKey::Property::Restitution.data()))
		{
			restitution = inJson[EngineKey::Property::Restitution.data()].get<float>();
		}
		if (inJson.contains(EngineKey::Property::IsSensor.data()))
		{
			isSensor = inJson[EngineKey::Property::IsSensor.data()].get<bool>();
		}
		if (inJson.contains(EngineKey::Property::BodyType.data()))
		{
			m_BodyType = 
				static_cast<b2BodyType>
				(inJson[EngineKey::Property::BodyType.data()].get<int>());
		}

		RebuildShape();
	}

	virtual void DrawDebug() {}

	virtual void Awake() override;
	void RebuildShape()
	{
		if (!b2Body_IsValid(m_BodyId)) return;

		if (b2Shape_IsValid(m_ShapeId))
		{
			b2DestroyShape(m_ShapeId, true);
			m_ShapeId = b2_nullShapeId;
		}

		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.density = this->density;
		shapeDef.isSensor = this->isSensor;

		shapeDef.filter.categoryBits = 0xFFFFFFFF;
		shapeDef.filter.maskBits = 0xFFFFFFFF;

		m_ShapeId = CreateShape(m_BodyId, &shapeDef);

		if (b2Shape_IsValid(m_ShapeId))
		{
			b2Shape_SetFriction(m_ShapeId, this->friction);
			b2Shape_SetRestitution(m_ShapeId, this->restitution);
		}

		if (GetBodyType() == b2_dynamicBody)
		{
			b2Body_ApplyMassFromShapes(m_BodyId);
		}
	}

	b2BodyId GetBodyId() const { return m_BodyId; }
	void SetBodyId(b2BodyId id) { m_BodyId = id; }

	b2ShapeId GetShapeId() const { return m_ShapeId; }
	void SetShapeId(b2ShapeId id) { m_ShapeId = id; }

	b2BodyType GetBodyType() const { return m_BodyType; }
	void SetBodyType(b2BodyType type)
	{
		if (m_BodyType == type)return;

		m_BodyType = type;

		if (b2Body_IsValid(m_BodyId))
		{
			b2Body_SetType(m_BodyId, type);

			RebuildShape();
		}
	}

	size_t GetPhysicsVectorIndex() const { return m_PhysicsVectorIndex; }
	void SetPhysicsVectorIndex(size_t idx) { m_PhysicsVectorIndex = idx; }

	float density = 1.0f;		//box2D 밀도
	float friction = 0.3f;		//  "   마찰력
	float restitution = 0.0f;	//  "   탄성력
	bool isSensor = false;

protected:
	virtual b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef* shapeDef) = 0;

	b2BodyId m_BodyId = b2_nullBodyId;
	b2ShapeId m_ShapeId = b2_nullShapeId;
	b2BodyType m_BodyType = b2_dynamicBody;

	size_t m_PhysicsVectorIndex = 0;
};