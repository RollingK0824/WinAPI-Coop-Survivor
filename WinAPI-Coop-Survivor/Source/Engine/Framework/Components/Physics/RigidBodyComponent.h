#pragma once
#include "Engine/Framework/Base/Component.h"
#include "Engine/Core/Define.h"

class RigidBodyComponent : public Component
{
	friend class PhysicsManager;
public:
	CLONEABLE_COMPONENT(RigidBodyComponent)

	RigidBodyComponent(GameObject* owner, TransformComponent* transform);
	virtual ~RigidBodyComponent() override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::RigidBody;
	}

	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void PostDeserialize(Scene* pScene) override;

	b2BodyId GetBodyId() const { return m_bodyId; }

	// Body 타입 (Dynamic / Static / Kinematic)
	b2BodyType GetBodyType() const { return m_bodyType; }
	void SetBodyType(b2BodyType type);

	bool IsFixedRotation() const { return m_bFixedRotation; }
	void SetFixedRotation(bool fixed);

	// 속도 제어
	void SetLinearVelocity(const Vector2& vel);
	Vector2 GetLinearVelocity() const;
	void SetAngularVelocity(float vel);
	float GetAngularVelocity() const;

	void ApplyForce(const Vector2& force);
	void ApplyImpulse(const Vector2& impulse);

	void SyncTransformFromBody();

	virtual void Serialize(json& outJson) const override;
	virtual void Deserialize(const json& inJson) override;

private:
	b2BodyId   m_bodyId      = b2_nullBodyId;
	b2BodyType m_bodyType    = b2_dynamicBody;
	bool       m_bFixedRotation = true;
};
