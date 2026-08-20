#include "Engine/Core/pch.h"
#include "PhysicsManager.h"
#include "Engine/Manager/ActionManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/ColliderComponent.h"
#include "Engine/Framework/Components/Physics/RigidBodyComponent.h"

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

	for (auto* pRigidBody : m_vRigidBodies)
	{
		if (pRigidBody == nullptr || !pRigidBody->IsEnabled()) continue;
		if (!pRigidBody->gameObject.IsActiveInHierarchy()) continue;
		if (pRigidBody->GetBodyType() == b2_staticBody) continue;

		pRigidBody->SyncTransformFromBody();
	}

	for (auto* pCollider : m_vColliders)
	{
		if (pCollider == nullptr || !pCollider->IsEnabled()) continue;
		if (pCollider->IsAttachedToRigidBody()) continue;
		if (!pCollider->gameObject.IsActiveInHierarchy()) continue;
		if (pCollider->GetBodyType() == b2_staticBody) continue;

		pCollider->SyncTransformFromBody();
	}
}

void PhysicsManager::Update(float dt)
{
}

static ColliderComponent* GetColliderFromShapeId(b2ShapeId shapeId)
{
	if (!b2Shape_IsValid(shapeId)) return nullptr;
	ColliderComponent* col = reinterpret_cast<ColliderComponent*>(b2Shape_GetUserData(shapeId));
	if (col) return col;

	b2BodyId bodyId = b2Shape_GetBody(shapeId);
	if (b2Body_IsValid(bodyId))
	{
		void* bodyUserData = b2Body_GetUserData(bodyId);
		if (bodyUserData)
		{
			Component* comp = reinterpret_cast<Component*>(bodyUserData);
			if (comp)
			{
				col = comp->gameObject.GetComponent<ColliderComponent>();
			}
		}
	}
	return col;
}

void PhysicsManager::ProcessContanctEvents()
{
	b2ContactEvents contactEvents = b2World_GetContactEvents(m_worldId);
	for (int i = 0; i < contactEvents.beginCount; ++i)
	{
		b2ContactBeginTouchEvent event = contactEvents.beginEvents[i];

		ColliderComponent* colA = GetColliderFromShapeId(event.shapeIdA);
		ColliderComponent* colB = GetColliderFromShapeId(event.shapeIdB);

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

		ColliderComponent* colVisitor = GetColliderFromShapeId(event.visitorShapeId);
		ColliderComponent* colSensor = GetColliderFromShapeId(event.sensorShapeId);

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

void PhysicsManager::RegisterRigidBody(RigidBodyComponent* pRigidBody)
{
	if (pRigidBody == nullptr) return;
	if (b2Body_IsValid(pRigidBody->GetBodyId())) return;

	TransformComponent* pTf = &pRigidBody->transform;

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type          = pRigidBody->GetBodyType();
	bodyDef.fixedRotation = pRigidBody->IsFixedRotation();
	bodyDef.position      = { PixelToMeter(pTf->GetWorldPosition().x), PixelToMeter(pTf->GetWorldPosition().y) };
	bodyDef.rotation      = b2MakeRot(DegreeToRadian(pTf->GetWorldRotation()));
	bodyDef.userData      = pRigidBody;

	bool shouldBeActive = pRigidBody->gameObject.IsActiveInHierarchy() && pRigidBody->IsEnabled();
	bodyDef.isEnabled = shouldBeActive;

	b2BodyId bodyId = CreateBody(&bodyDef);
	pRigidBody->m_bodyId = bodyId;

	if (!shouldBeActive)
		b2Body_Disable(bodyId);

	m_vRigidBodies.push_back(pRigidBody);

	// 만약 이 오브젝트나 자식 오브젝트의 Collider들이 이미 등록되어 있다면 해당 Collider들을 이 Body로 재연결
	for (auto* pCollider : m_vColliders)
	{
		if (pCollider == nullptr) continue;
		if (pCollider->gameObject.GetComponentInParent<RigidBodyComponent>() == pRigidBody ||
			pCollider->gameObject.GetComponent<RigidBodyComponent>() == pRigidBody)
		{
			if (!pCollider->IsAttachedToRigidBody())
			{
				if (b2Body_IsValid(pCollider->GetBodyId()))
				{
					DestoryBody(pCollider->GetBodyId());
				}
				pCollider->SetBodyId(bodyId);
				pCollider->SetAttachedToRigidBody(true);
				pCollider->RebuildShape();
			}
		}
	}
}

void PhysicsManager::UnRegisterRigidBody(RigidBodyComponent* pRigidBody)
{
	if (pRigidBody == nullptr) return;

	if (b2Body_IsValid(pRigidBody->m_bodyId))
	{
		b2DestroyBody(pRigidBody->m_bodyId);
		pRigidBody->m_bodyId = b2_nullBodyId;
	}

	auto it = std::find(m_vRigidBodies.begin(), m_vRigidBodies.end(), pRigidBody);
	if (it != m_vRigidBodies.end())
		m_vRigidBodies.erase(it);
}

void PhysicsManager::RegisterCollider(ColliderComponent* pCollider)
{
	if (pCollider == nullptr) return;

	RigidBodyComponent* pRigidBody = pCollider->gameObject.GetComponentInParent<RigidBodyComponent>();
	if (pRigidBody == nullptr)
		pRigidBody = pCollider->gameObject.GetComponent<RigidBodyComponent>();

	b2BodyId bodyId = b2_nullBodyId;

	if (pRigidBody != nullptr)
	{
		// RigidBody가 아직 초기화되지 않았다면 먼저 Body 생성을 보장
		if (!b2Body_IsValid(pRigidBody->GetBodyId()))
		{
			RegisterRigidBody(pRigidBody);
		}

		bodyId = pRigidBody->GetBodyId();
		pCollider->SetBodyId(bodyId);
		pCollider->SetAttachedToRigidBody(true);
	}
	else
	{
		pCollider->SetAttachedToRigidBody(false);
		if (b2Body_IsValid(pCollider->GetBodyId())) return;

		TransformComponent* pTf = &pCollider->transform;

		b2BodyDef bodyDef = b2DefaultBodyDef();
		bodyDef.type          = pCollider->GetBodyType();
		bodyDef.fixedRotation = pCollider->IsFixedRotation();
		bodyDef.position      = { PixelToMeter(pTf->GetWorldPosition().x), PixelToMeter(pTf->GetWorldPosition().y) };
		bodyDef.rotation      = b2MakeRot(DegreeToRadian(pTf->GetWorldRotation()));
		bodyDef.userData      = pCollider;

		bool shouldBeActive = pCollider->gameObject.IsActiveInHierarchy() && pCollider->IsEnabled();
		bodyDef.isEnabled = shouldBeActive;

		bodyId = CreateBody(&bodyDef);
		pCollider->SetBodyId(bodyId);

		if (!shouldBeActive)
			b2Body_Disable(bodyId);
	}

	pCollider->SetPhysicsVectorIndex(m_vColliders.size());
	m_vColliders.push_back(pCollider);
	pCollider->RebuildShape();
}

void PhysicsManager::UnRegisterCollider(ColliderComponent* pCollider)
{
	if (pCollider == nullptr) return;

	if (b2Shape_IsValid(pCollider->GetShapeId()))
	{
		b2DestroyShape(pCollider->GetShapeId(), false);
		pCollider->SetShapeId(b2_nullShapeId);
	}

	if (!pCollider->IsAttachedToRigidBody())
	{
		if (b2Body_IsValid(pCollider->GetBodyId()))
		{
			b2DestroyBody(pCollider->GetBodyId());
		}
	}
	pCollider->SetBodyId(b2_nullBodyId);
	pCollider->SetAttachedToRigidBody(false);

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
	ColliderComponent* col = GetColliderFromShapeId(shapeId);
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
