#include "Engine/Core/pch.h"
#include "TransformComponent.h"
#include "Engine/Framework/GameObject.h"

TransformComponent::TransformComponent(GameObject* owner) :Component(owner)
{
	ExposeVariable("Position X", &m_Position.x);
	ExposeVariable("Position Y", &m_Position.y);

	ExposeVariable("Scale X", &m_Scale.x);
	ExposeVariable("Scale Y", &m_Scale.y);

	ExposeVariable("Rotation", &m_Rotation.angle);
}

void TransformComponent::SetSiblingIndex(int index)
{
	gameObject.SetSiblingIndex(index);
}
int TransformComponent::GetSiblingIndex() const
{
	return gameObject.GetSiblingIndex();
}
void TransformComponent::SetAsFirstSibling()
{
	gameObject.SetAsFirstSibling();
}
void TransformComponent::SetAsLastSibling()
{
	gameObject.SetAsLastSibling();
}