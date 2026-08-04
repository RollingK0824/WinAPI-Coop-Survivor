#include "Engine/Core/pch.h"
#include "PhysicsManager.h"
#include "Engine/Manager/ActionManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/ColliderComponent.h"

bool PhysicsManager::Initialize()
{
	b2WorldDef worldDef = b2DefaultWorldDef();
	b2Vec2 gravity{ 0.0f, 9.8f };
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
	if (ActionManager::GetInstance()->GetActionDown("Debug_Mode"))
	{
		m_bEnableDebugDraw = !m_bEnableDebugDraw;
	}

	// 디버그 모드가 켜져 있다면 모든 콜라이더에게 그리기 명령 제출 지시
	if (m_bEnableDebugDraw)
	{
		for (auto* pCollider : m_vColliders)
		{
			if (pCollider && pCollider->IsEnabled() && pCollider->gameObject.IsActive())
			{
				pCollider->DrawDebug();
			}
		}
	}
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
			colA->OnCollision(colB);
			colB->OnCollision(colA);
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
	if (pCollider == nullptr)return;

	TransformComponent* pTransform = &pCollider->transform;
	if (pTransform == nullptr)return;

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = pCollider->GetBodyType();

	bodyDef.position = b2Vec2{ PixelToMeter(pTransform->GetPosition().x),PixelToMeter(pTransform->GetPosition().y) };
	bodyDef.rotation = b2MakeRot(pTransform->GetRotation().angle);

	bodyDef.userData = pCollider;

	b2BodyId bodyId = CreateBody(&bodyDef);
	pCollider->SetBodyId(bodyId);

	pCollider->SetPhysicsVectorIndex(m_vColliders.size());
	m_vColliders.push_back(pCollider);
}

void PhysicsManager::UnRegisterCollider(ColliderComponent* pCollider)
{
	if (pCollider == nullptr)return;

	if (b2Body_IsValid(pCollider->GetBodyId()))
	{
		b2DestroyBody(pCollider->GetBodyId());
	}

	size_t deleteIdx = pCollider->GetPhysicsVectorIndex();
	size_t lastIdx = m_vColliders.size() - 1;

	if (deleteIdx != lastIdx)
	{
		m_vColliders[deleteIdx] = m_vColliders[lastIdx];
		m_vColliders[deleteIdx]->SetPhysicsVectorIndex(deleteIdx);
	}

	m_vColliders.pop_back();
}

