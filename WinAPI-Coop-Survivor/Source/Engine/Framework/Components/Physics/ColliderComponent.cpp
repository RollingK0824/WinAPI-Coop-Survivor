#include "Engine/Core/pch.h"
#include "ColliderComponent.h"
#include "Engine/Physics/PhysicsManager.h"

ColliderComponent::ColliderComponent(GameObject* owner, TransformComponent* transform)
	: Component(owner, transform)
{
	ExposeVariable("Density", &m_density);
	ExposeVariable("IsSensor", &m_bIsSensor);
	ExposeVariable("BodyType", reinterpret_cast<int*>(&m_BodyType));
}

ColliderComponent::~ColliderComponent()
{
	PhysicsManager::GetInstance()->UnRegisterCollider(this);
}

void ColliderComponent::Awake()
{
	PhysicsManager::GetInstance()->RegisterCollider(this);
	RebuildShape();
}

void ColliderComponent::PostDeserialize(Scene* pScene)
{
	Component::PostDeserialize(pScene);
	RebuildShape();
}

void ColliderComponent::OnEnable()
{
	if (b2Body_IsValid(m_BodyId))
	{
		b2Body_Enable(m_BodyId);
	}
}

void ColliderComponent::OnDisable()
{
	if (b2Body_IsValid(m_BodyId))
	{
		b2Body_Disable(m_BodyId);
	}
}

void ColliderComponent::SetBodyType(b2BodyType type)
{
	if (m_BodyType == type) return;

	m_BodyType = type;

	if (b2Body_IsValid(m_BodyId))
	{
		b2Body_SetType(m_BodyId, type);
		RebuildShape();
	}
}

void ColliderComponent::RebuildShape()
{
	if (!b2Body_IsValid(m_BodyId)) return;

	if (b2Shape_IsValid(m_ShapeId))
	{
		b2DestroyShape(m_ShapeId, true);
		m_ShapeId = b2_nullShapeId;
	}

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = m_density;
	shapeDef.isSensor = m_bIsSensor;

	shapeDef.filter.categoryBits = 0xFFFFFFFF;
	shapeDef.filter.maskBits = 0xFFFFFFFF;

	m_ShapeId = CreateShape(m_BodyId, &shapeDef);

	if (GetBodyType() == b2_dynamicBody)
	{
		b2Body_ApplyMassFromShapes(m_BodyId);
	}
}