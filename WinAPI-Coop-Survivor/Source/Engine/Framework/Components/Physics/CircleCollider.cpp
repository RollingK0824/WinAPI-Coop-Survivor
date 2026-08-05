#include "Engine/Core/pch.h"
#include "CircleCollider.h"
#include "Engine/Core/ComponentRegister.h"

static ComponentRegistrar<CircleCollider> registrar(EngineKey::Component::CircleCollider.data());

CircleCollider::CircleCollider(GameObject* owner, TransformComponent* transform)
	: ColliderComponent(owner, transform)
{
	ExposeVariable("Radius", &m_Radius);
}