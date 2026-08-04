#include "Engine/Core/pch.h"
#include "ColliderComponent.h"
#include "Engine/Physics/PhysicsManager.h"

ColliderComponent::ColliderComponent(GameObject* owner, TransformComponent* transform)
	:Component(owner, transform)
{
	ExposeVariable("Density", &density);
	ExposeVariable("Friction", &friction);
	ExposeVariable("Restitution", &restitution);
	ExposeVariable("Is Sensor", &isSensor);

	ExposeVariable("Body Type (0:Static, 1:Kinematic, 2:Dynamic)", reinterpret_cast<int*>(&m_BodyType));
}

void ColliderComponent::Awake()
{
	PhysicsManager::GetInstance()->RegisterCollider(this);

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