#include "Engine/Core/pch.h"
#include "PhysicsManager.h"
#include "Engine/Manager/ActionManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/ColliderComponent.h"

bool PhysicsManager::Initialize()
{
	b2WorldDef worldDef = b2DefaultWorldDef();
	b2Vec2 gravity{ 0.0f, 0.0f };
	worldDef.gravity = gravity;

	m_worldId = b2CreateWorld(&worldDef);
	if (!b2World_IsValid(m_worldId))return false;

	return b2World_IsValid(m_worldId);
}

void PhysicsManager::Release()
{
	if (b2World_IsValid(m_worldId))
	{
		b2DestroyWorld(m_worldId);
		m_worldId = b2_nullWorldId;
	}
}

void PhysicsManager::FixedUpdate(float fixedDt)
{
	if (!b2World_IsValid(m_worldId))return;

	b2World_Step(m_worldId, fixedDt, m_subStepCount);

	ProcessContanctEvents();

	for (auto* pCollider : m_vColliders)
	{
		if (pCollider == nullptr || !pCollider->IsEnabled())continue;

		GameObject* pOwner = &pCollider->gameObject;
		if (pOwner == nullptr || !pOwner->IsActive()) continue;

		if (pCollider->GetBodyType() == b2_staticBody) continue;

		b2BodyId bodyId = pCollider->GetBodyId();
		if (!b2Body_IsValid(bodyId))continue;

		b2Vec2 b2Pos = b2Body_GetPosition(bodyId);
		b2Rot b2Rot = b2Body_GetRotation(bodyId);

		TransformComponent* pTransform = &pOwner->transform;
		if (pTransform != nullptr)
		{
			pTransform->SetPosition(MeterToPixel(b2Pos.x), MeterToPixel(b2Pos.y));
			float angleRadian = b2Rot_GetAngle(b2Rot);
			pTransform->SetRotation(RadianToDegree(angleRadian));
		}
	}
}

void PhysicsManager::Update(float dt)
{
}

void PhysicsManager::ProcessContanctEvents()
{
	b2ContactEvents contactEvents = b2World_GetContactEvents(m_worldId);
	for (int i = 0; i < contactEvents.beginCount; ++i)
	{
		b2ContactBeginTouchEvent event = contactEvents.beginEvents[i];

		b2BodyId bodyA = b2Shape_GetBody(event.shapeIdA);
		b2BodyId bodyB = b2Shape_GetBody(event.shapeIdB);

		ColliderComponent* colA = reinterpret_cast<ColliderComponent*>(b2Body_GetUserData(bodyA));
		ColliderComponent* colB = reinterpret_cast<ColliderComponent*>(b2Body_GetUserData(bodyB));

		if (colA && colB 
			&& colA->IsEnabled() && colB->IsEnabled()
			&& colA->gameObject.IsActive() && colB->gameObject.IsActive())
		{
			colA->gameObject.OnCollision(colB);
			colB->gameObject.OnCollision(colA);
		}
	}

	b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_worldId);
	for (int i = 0; i < sensorEvents.beginCount; ++i)
	{
		b2SensorBeginTouchEvent event = sensorEvents.beginEvents[i];

		b2BodyId bodyVisitor = b2Shape_GetBody(event.visitorShapeId);
		b2BodyId bodySensor = b2Shape_GetBody(event.sensorShapeId);

		ColliderComponent* colVisitor = reinterpret_cast<ColliderComponent*>(b2Body_GetUserData(bodyVisitor));
		ColliderComponent* colSensor = reinterpret_cast<ColliderComponent*>(b2Body_GetUserData(bodySensor));

		if (colVisitor && colSensor 
			&& colVisitor->IsEnabled() && colSensor->IsEnabled()
			&& colVisitor->gameObject.IsActive() && colSensor->gameObject.IsActive())
		{
			colVisitor->gameObject.OnCollision(colSensor);
			colSensor->gameObject.OnCollision(colVisitor);
		}
	}
}
b2BodyId PhysicsManager::CreateBody(const b2BodyDef* def)
{
	return b2CreateBody(m_worldId, def);
}

void PhysicsManager::DestoryBody(b2BodyId bodyId)
{
	if (b2Body_IsValid(bodyId))
	{
		b2DestroyBody(bodyId);
	}
}

void PhysicsManager::RegisterCollider(ColliderComponent* pCollider)
{
	if (pCollider == nullptr) return;

	if (b2Body_IsValid(pCollider->GetBodyId())) return;

	TransformComponent* pTransform = &pCollider->transform;
	if (pTransform == nullptr) return;

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = pCollider->GetBodyType();
	bodyDef.fixedRotation = pCollider->IsFixedRotation();

	bodyDef.position = b2Vec2{ PixelToMeter(pTransform->GetPosition().x), PixelToMeter(pTransform->GetPosition().y) };
	bodyDef.rotation = b2MakeRot(pTransform->GetRotation().angle);

	bodyDef.userData = pCollider;

	// Set isEnabled on BodyDef directly to prevent 1-frame overlap impulse during body creation
	bool shouldBeActive = pCollider->gameObject.IsActive() && pCollider->IsEnabled();
	bodyDef.isEnabled = shouldBeActive;

	b2BodyId bodyId = CreateBody(&bodyDef);
	pCollider->SetBodyId(bodyId);

	if (!shouldBeActive)
	{
		b2Body_Disable(bodyId);
	}

	pCollider->SetPhysicsVectorIndex(m_vColliders.size());
	m_vColliders.push_back(pCollider);
}

void PhysicsManager::UnRegisterCollider(ColliderComponent* pCollider)
{
	if (pCollider == nullptr) return;

	if (b2Body_IsValid(pCollider->GetBodyId()))
	{
		b2DestroyBody(pCollider->GetBodyId());
		pCollider->SetBodyId(b2_nullBodyId);
	}

	if (m_vColliders.empty()) return;

	size_t deleteIdx = pCollider->GetPhysicsVectorIndex();
	if (deleteIdx >= m_vColliders.size() || m_vColliders[deleteIdx] != pCollider)
	{
		auto it = std::find(m_vColliders.begin(), m_vColliders.end(), pCollider);
		if (it == m_vColliders.end()) return;
		deleteIdx = std::distance(m_vColliders.begin(), it);
	}

	size_t lastIdx = m_vColliders.size() - 1;

	if (deleteIdx != lastIdx)
	{
		m_vColliders[deleteIdx] = m_vColliders[lastIdx];
		m_vColliders[deleteIdx]->SetPhysicsVectorIndex(deleteIdx);
	}

	m_vColliders.pop_back();
	pCollider->SetPhysicsVectorIndex((size_t)-1);
}

struct OverlapContext
{
	std::vector<ColliderComponent*>* results = nullptr;
	uint32 maskBits = PhysicsLayer::All;
};

static bool OverlapCallback(b2ShapeId shapeId, void* context)
{
	b2BodyId bodyId = b2Shape_GetBody(shapeId);
	ColliderComponent* col = reinterpret_cast<ColliderComponent*>(b2Body_GetUserData(bodyId));
	if (col && col->IsEnabled() && col->gameObject.IsActive())
	{
		OverlapContext* ctx = static_cast<OverlapContext*>(context);
		if (ctx && ctx->results)
		{
			if (ctx->maskBits == PhysicsLayer::All || (col->GetCategoryBits() & ctx->maskBits) != 0 || col->GetCategoryBits() == PhysicsLayer::Default)
			{
				ctx->results->push_back(col);
			}
		}
	}
	return true;
}

std::vector<ColliderComponent*> PhysicsManager::OverlapAABB(const Vector2& center, float radius, uint32 maskBits)
{
	std::vector<ColliderComponent*> results;
	if (!b2World_IsValid(m_worldId)) return results;

	OverlapContext ctx;
	ctx.results = &results;
	ctx.maskBits = maskBits;

	b2Vec2 centerMeter = { PixelToMeter(center.x), PixelToMeter(center.y) };
	float radiusMeter = PixelToMeter(radius);

	b2AABB aabb;
	aabb.lowerBound = { centerMeter.x - radiusMeter, centerMeter.y - radiusMeter };
	aabb.upperBound = { centerMeter.x + radiusMeter, centerMeter.y + radiusMeter };

	b2QueryFilter filter = b2DefaultQueryFilter();
	filter.categoryBits = PhysicsLayer::All;
	filter.maskBits = PhysicsLayer::All;

	b2World_OverlapAABB(m_worldId, aabb, filter, OverlapCallback, &ctx);
	return results;
}
