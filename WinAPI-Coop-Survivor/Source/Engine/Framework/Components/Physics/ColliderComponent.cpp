#include "Engine/Core/pch.h"
#include "ColliderComponent.h"
#include "Engine/Physics/PhysicsManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"

ColliderComponent::ColliderComponent(GameObject* owner, TransformComponent* transform)
	: Component(owner, transform)
{
	ExposeVariable("Offset", &m_offset);
	ExposeVariable("Density", &m_density);
	ExposeVariable("IsSensor", &m_bIsSensor);
	ExposeVariable("BodyType", reinterpret_cast<int*>(&m_BodyType));
	ExposeVariable("FixedRotation", &m_bFixedRotation);
	ExposeVariable("CategoryBits", reinterpret_cast<int*>(&m_categoryBits));
	ExposeVariable("MaskBits", reinterpret_cast<int*>(&m_maskBits));
}

ColliderComponent::~ColliderComponent()
{
	PhysicsManager::GetInstance()->UnRegisterCollider(this);
}

void ColliderComponent::Awake()
{
	PhysicsManager::GetInstance()->RegisterCollider(this);
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

void ColliderComponent::SetFixedRotation(bool fixed)
{
	m_bFixedRotation = fixed;
	if (b2Body_IsValid(m_BodyId))
	{
		b2Body_SetFixedRotation(m_BodyId, fixed);
	}
}

void ColliderComponent::SetFilter(uint32 categoryBits, uint32 maskBits)
{
	m_categoryBits = categoryBits;
	m_maskBits = maskBits;

	if (b2Shape_IsValid(m_ShapeId))
	{
		b2Filter filter = GetFilter();
		b2Shape_SetFilter(m_ShapeId, filter);
	}
}

b2Filter ColliderComponent::GetFilter() const
{
	b2Filter filter = b2DefaultFilter();
	filter.categoryBits = m_categoryBits;
	filter.maskBits = m_maskBits;
	return filter;
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
	shapeDef.enableContactEvents = true;
	shapeDef.enableSensorEvents = true;
	shapeDef.filter = GetFilter();
	shapeDef.userData = this;

	m_ShapeId = CreateShape(m_BodyId, &shapeDef);
	if (b2Shape_IsValid(m_ShapeId))
	{
		b2Shape_SetUserData(m_ShapeId, this);
	}

	if (GetBodyType() == b2_dynamicBody)
	{
		b2Body_ApplyMassFromShapes(m_BodyId);
	}
}

void ColliderComponent::SyncTransformFromBody()
{
	if (!b2Body_IsValid(m_BodyId) || m_bAttachedToRigidBody) return;

	b2Vec2 b2Pos = b2Body_GetPosition(m_BodyId);
	b2Rot  b2Rot = b2Body_GetRotation(m_BodyId);

	Vector2 worldPos(MeterToPixel(b2Pos.x), MeterToPixel(b2Pos.y));
	float   worldRot = RadianToDegree(b2Rot_GetAngle(b2Rot));

	TransformComponent& tf = transform;
	if (gameObject.GetParent() != nullptr)
	{
		Vector2 localPos = tf.WorldToLocal(worldPos);
		float parentWorldRot = gameObject.GetParent()->transform.GetWorldRotation();
		tf.SetLocalPosition(localPos);
		tf.SetLocalRotation(worldRot - parentWorldRot);
	}
	else
	{
		tf.SetLocalPosition(worldPos);
		tf.SetLocalRotation(worldRot);
	}
}