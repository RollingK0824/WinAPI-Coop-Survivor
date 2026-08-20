#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Core/Define.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IUpdatable.h"

class ColliderComponent;
class RigidBodyComponent;

class PhysicsManager : public Singleton<PhysicsManager>, public ISystem, public IUpdatable
{
	friend class Singleton<PhysicsManager>;

public:
	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void FixedUpdate(float fixedDt) override;
	virtual void Update(float dt)override;

	b2BodyId CreateBody(const b2BodyDef* def);
	void DestoryBody(b2BodyId bodyId);

	void RegisterRigidBody(RigidBodyComponent* pRigidBody);
	void UnRegisterRigidBody(RigidBodyComponent* pRigidBody);

	void RegisterCollider(ColliderComponent* pCollider);
	void UnRegisterCollider(ColliderComponent* pCollider);

	const std::vector<ColliderComponent*>& GetColliders() const { return m_vColliders; }

	std::vector<ColliderComponent*> OverlapAABB(const Vector2& center, float radius, uint32 maskBits = PhysicsLayer::All);

private:
	PhysicsManager() = default;
	virtual ~PhysicsManager() = default;

private:
	void ProcessContanctEvents();

	b2WorldId m_worldId = b2_nullWorldId;
	int32_t m_subStepCount = 4;
	bool m_bEnableDebugDraw = false;

	std::vector<ColliderComponent*>   m_vColliders;
	std::vector<RigidBodyComponent*>  m_vRigidBodies;
};

