#include "Engine/Core/pch.h"
#include "RigidBodyComponent.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Physics/PhysicsManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"

static ComponentRegistrar<RigidBodyComponent> registrar(EngineKey::Component::RigidBody.data());

RigidBodyComponent::RigidBodyComponent(GameObject* owner, TransformComponent* pTransform)
	: Component(owner, pTransform)
{
	ExposeVariable("BodyType",      reinterpret_cast<int*>(&m_bodyType));
	ExposeVariable("FixedRotation", &m_bFixedRotation);
}

RigidBodyComponent::~RigidBodyComponent()
{
	PhysicsManager::GetInstance()->UnRegisterRigidBody(this);
}

void RigidBodyComponent::Awake()
{
	PhysicsManager::GetInstance()->RegisterRigidBody(this);
}

void RigidBodyComponent::OnEnable()
{
	if (b2Body_IsValid(m_bodyId))
		b2Body_Enable(m_bodyId);
}

void RigidBodyComponent::OnDisable()
{
	if (b2Body_IsValid(m_bodyId))
		b2Body_Disable(m_bodyId);
}

void RigidBodyComponent::PostDeserialize(Scene* pScene)
{
	Component::PostDeserialize(pScene);
}

void RigidBodyComponent::SetBodyType(b2BodyType type)
{
	if (m_bodyType == type) return;
	m_bodyType = type;
	if (b2Body_IsValid(m_bodyId))
		b2Body_SetType(m_bodyId, type);
}

void RigidBodyComponent::SetFixedRotation(bool fixed)
{
	m_bFixedRotation = fixed;
	if (b2Body_IsValid(m_bodyId))
		b2Body_SetFixedRotation(m_bodyId, fixed);
}

void RigidBodyComponent::SetLinearVelocity(const Vector2& vel)
{
	if (b2Body_IsValid(m_bodyId))
		b2Body_SetLinearVelocity(m_bodyId, { vel.x, vel.y });
}

Vector2 RigidBodyComponent::GetLinearVelocity() const
{
	if (!b2Body_IsValid(m_bodyId)) return {};
	b2Vec2 v = b2Body_GetLinearVelocity(m_bodyId);
	return { v.x, v.y };
}

void RigidBodyComponent::SetAngularVelocity(float vel)
{
	if (b2Body_IsValid(m_bodyId))
		b2Body_SetAngularVelocity(m_bodyId, vel);
}

float RigidBodyComponent::GetAngularVelocity() const
{
	if (!b2Body_IsValid(m_bodyId)) return 0.0f;
	return b2Body_GetAngularVelocity(m_bodyId);
}

void RigidBodyComponent::ApplyForce(const Vector2& force)
{
	if (b2Body_IsValid(m_bodyId))
		b2Body_ApplyForceToCenter(m_bodyId, { force.x, force.y }, true);
}

void RigidBodyComponent::ApplyImpulse(const Vector2& impulse)
{
	if (b2Body_IsValid(m_bodyId))
		b2Body_ApplyLinearImpulseToCenter(m_bodyId, { impulse.x, impulse.y }, true);
}

void RigidBodyComponent::SyncTransformFromBody()
{
	if (!b2Body_IsValid(m_bodyId)) return;

	b2Vec2 b2Pos = b2Body_GetPosition(m_bodyId);
	b2Rot  b2Rot = b2Body_GetRotation(m_bodyId);

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

void RigidBodyComponent::Serialize(json& outJson) const
{
	Component::Serialize(outJson);
	outJson["BodyType"]      = static_cast<int>(m_bodyType);
	outJson["FixedRotation"] = m_bFixedRotation;
}

void RigidBodyComponent::Deserialize(const json& inJson)
{
	Component::Deserialize(inJson);
	if (inJson.contains("BodyType"))
		m_bodyType = static_cast<b2BodyType>(inJson["BodyType"].get<int>());
	if (inJson.contains("FixedRotation"))
		m_bFixedRotation = inJson["FixedRotation"].get<bool>();
}
