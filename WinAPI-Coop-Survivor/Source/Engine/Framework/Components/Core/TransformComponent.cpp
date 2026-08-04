#include "Engine/Core/pch.h"
#include "TransformComponent.h"

TransformComponent::TransformComponent(GameObject* owner) :Component(owner)
{
	ExposeVariable("Position X", &m_Position.x);
	ExposeVariable("Position Y", &m_Position.y);

	ExposeVariable("Scale X", &m_Scale.x);
	ExposeVariable("Scale Y", &m_Scale.y);

	ExposeVariable("Rotation", &m_Rotation.angle);
}
