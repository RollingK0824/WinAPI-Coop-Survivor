#include "Engine/Core/pch.h"
#include "GameObject.h"
#include "Engine/Editor/EditorSystem.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/Base/Component.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"

GameObject::GameObject(Scene* pOwnerScene)
	: m_pOwnerScene(pOwnerScene)
	, m_bIsActive(true)
	, m_bIsDead(false)
	, transform(*(new TransformComponent(this)))
{
	m_pTransform = &transform;
}

GameObject::~GameObject()
{
	if (EditorSystem::GetInstance()->GetSelectedObject() == this)
	{
		EditorSystem::GetInstance()->SetSelectedObject(nullptr);
	}

	for (Component* pComp : m_vComponents)
	{
		if (pComp == nullptr) continue;

		pComp->OnDestroy();
	}
	for (Component* pComp : m_vComponents)
	{
		if (pComp == nullptr) continue;
		delete pComp;
	}
	m_vComponents.clear();

	if (m_pTransform != nullptr)
	{
		delete m_pTransform;
		m_pTransform = nullptr;
	}
}

GameObject* GameObject::Clone(Scene* pNewOwnerScene)
{
	if (pNewOwnerScene == nullptr) return nullptr;

	GameObject* clone = pNewOwnerScene->CreateGameObject();
	clone->SetName(this->GetName());
	clone->SetActive(this->IsActive());
	
	clone->m_pTransform->SetPosition(this->m_pTransform->GetPosition());
	clone->m_pTransform->SetRotation(this->m_pTransform->GetRotation());
	clone->m_pTransform->SetScale(this->m_pTransform->GetScale());

	for (Component* comp : m_vComponents)
	{
		if (comp == nullptr || comp == m_pTransform) continue;

		Component* clonedComp = comp->Clone();
		clonedComp->BindToNewObject(clone, clone->m_pTransform);
		clone->m_vComponents.push_back(clonedComp);
		
		clone->RegisterComponentToScene(clonedComp);
	}

	return clone;
}

void GameObject::OnCollision(ColliderComponent* other)
{
	if (!m_bIsActive)return;
	if (other == nullptr)return;

	for (auto* pComponent : m_vComponents)
	{
		if (pComponent && pComponent->IsEnabled())
		{
			pComponent->OnCollision(other);
		}
	}
}

void GameObject::Destroy()
{
	if (m_bIsDead) return;

	m_bIsDead = true;

	if (m_pOwnerScene)
	{
		m_pOwnerScene->DestroyObjects(this);
	}
}

void GameObject::RegisterComponentToScene(Component* comp)
{
	if (m_pOwnerScene != nullptr)
	{
		m_pOwnerScene->OnComponentAdded(comp);
	}
}

void GameObject::RemoveComponent(Component* comp)
{
	if (comp == nullptr || comp == m_pTransform) return;
	auto it = std::find(m_vComponents.begin(), m_vComponents.end(), comp);
	if (it != m_vComponents.end())
	{
		comp->OnDestroy();
		delete comp;
		m_vComponents.erase(it);
	}
}

void GameObject::Serialize(json& outJson)const
{
	outJson[EngineKey::Property::IsActive.data()] = m_bIsActive;
	outJson[EngineKey::Property::Components.data()] = std::vector<json>();

	for (auto* comp : m_vComponents)
	{
		if (!comp)continue;

		json compJson;
		compJson[EngineKey::Property::Type.data()] = comp->GetComponentType().data();

		json compData;
		comp->Serialize(compData);
		compJson[EngineKey::Property::Data.data()] = compData;

		outJson[EngineKey::Property::Components.data()].push_back(compJson);
	}
}

void GameObject::SetActive(bool active)
{
	if (m_bIsActive == active) return;

	m_bIsActive = active;

	for (auto* comp : m_vComponents)
	{
		if (comp && comp->IsEnabled())
		{
			if (m_bIsActive)
			{
				comp->OnEnable();
			}
			else
			{
				comp->OnDisable();
			}
		}
	}
}
