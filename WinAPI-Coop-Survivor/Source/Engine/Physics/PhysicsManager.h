#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IUpdatable.h"

class ColliderComponent;

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

	void RegisterCollider(ColliderComponent* pCollider);
	void UnRegisterCollider(ColliderComponent* pCollider);

private:
	PhysicsManager() = default;
	virtual ~PhysicsManager() = default;

private:
	void ProcessContanctEvents();

	b2WorldId m_worldId = b2_nullWorldId;
	int32_t m_subStepCount = 4;
	bool m_bEnableDebugDraw = false;

	std::vector<ColliderComponent*> m_vColliders;


};

