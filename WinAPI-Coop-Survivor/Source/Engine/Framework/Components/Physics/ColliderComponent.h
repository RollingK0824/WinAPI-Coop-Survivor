#pragma once
#include "Engine/Framework/Base/Component.h"
#include "Engine/Core/Define.h"

class ColliderComponent : public Component
{
public:
	ColliderComponent(GameObject* owner, TransformComponent* transform);
	virtual ~ColliderComponent() override;

	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void Awake() override;
	virtual void PostDeserialize(Scene* pScene) override;

	virtual void DrawDebug() {}

	void RebuildShape();

	b2BodyId GetBodyId() const { return m_BodyId; }
	void SetBodyId(b2BodyId id) { m_BodyId = id; }

	b2ShapeId GetShapeId() const { return m_ShapeId; }
	void SetShapeId(b2ShapeId id) { m_ShapeId = id; }

	b2BodyType GetBodyType() const { return m_BodyType; }
	void SetBodyType(b2BodyType type);

	size_t GetPhysicsVectorIndex() const { return m_PhysicsVectorIndex; }
	void SetPhysicsVectorIndex(size_t idx) { m_PhysicsVectorIndex = idx; }

	void SetFixedRotation(bool fixed);
	bool IsFixedRotation() const { return m_bFixedRotation; }

	void SetFilter(uint32 categoryBits, uint32 maskBits);
	uint32 GetCategoryBits() const { return m_categoryBits; }
	uint32 GetMaskBits() const { return m_maskBits; }
	b2Filter GetFilter() const;

	float m_density = 1.0f;
	bool m_bIsSensor = false;

protected:
	virtual b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef* shapeDef) = 0;

	b2BodyId m_BodyId = b2_nullBodyId;
	b2ShapeId m_ShapeId = b2_nullShapeId;
	b2BodyType m_BodyType = b2_dynamicBody;

	bool m_bFixedRotation = true;
	uint32 m_categoryBits = PhysicsLayer::Default;
	uint32 m_maskBits = PhysicsLayer::All;

	size_t m_PhysicsVectorIndex = 0;
};