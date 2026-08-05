#include "Engine/Core/pch.h"
#include "TransformComponent.h"
#include "Engine/Framework/GameObject.h"

TransformComponent::TransformComponent(GameObject* owner)
	: Component(owner)
{
	ExposeVariable("Position", &m_Position);
	ExposeVariable("Rotation", &m_Rotation.angle);
	ExposeVariable("Scale", &m_Scale);
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