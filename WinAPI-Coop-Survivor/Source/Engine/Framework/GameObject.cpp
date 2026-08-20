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
	m_instanceID = s_nextInstanceID++;
	m_pTransform = &transform;
}

GameObject::~GameObject()
{
	m_bIsDead = true;

	for (void** observerPtr : m_vObservers)
	{
		if (observerPtr)
		{
			*observerPtr = nullptr;
		}
	}
	m_vObservers.clear();

	if (EditorSystem::GetInstance()->GetSelectedObject() == this)
	{
		EditorSystem::GetInstance()->SetSelectedObject(nullptr);
	}

	if (m_pParent != nullptr)
	{
		auto& siblings = m_pParent->m_vChildren;
		siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
		m_pParent = nullptr;
	}

	std::vector<GameObject*> childrenToDelete = m_vChildren;
	m_vChildren.clear();

	for (GameObject* pChild : childrenToDelete)
	{
		if (pChild != nullptr)
		{
			pChild->m_pParent = nullptr; 
			delete pChild;
		}
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
	
	clone->m_pTransform->SetLocalPosition(this->m_pTransform->GetLocalPosition());
	clone->m_pTransform->SetLocalRotation(this->m_pTransform->GetLocalRotation());
	clone->m_pTransform->SetLocalScale(this->m_pTransform->GetLocalScale());

	for (Component* comp : m_vComponents)
	{
		if (comp == nullptr || comp == m_pTransform) continue;

		Component* clonedComp = comp->Clone();
		clonedComp->BindToNewObject(clone, clone->m_pTransform);
		clone->m_vComponents.push_back(clonedComp);
		
		clone->RegisterComponentToScene(clonedComp);
	}

	for (GameObject* pChild : m_vChildren)
	{
		if (pChild == nullptr) continue;
		GameObject* clonedChild = pChild->Clone(pNewOwnerScene);
		clonedChild->SetParent(clone, false);
	}

	return clone;
}

void GameObject::OnCollision(ColliderComponent* other)
{
	if (!m_bIsActive) return;
	if (other == nullptr) return;

	for (auto* pComponent : m_vComponents)
	{
		if (pComponent && pComponent->IsEnabled())
		{
			pComponent->OnCollision(other);
		}
	}

	if (m_pParent != nullptr)
	{
		m_pParent->OnCollision(other);
	}
}

void GameObject::Destroy()
{
	if (m_bIsDead) return;

	m_bIsDead = true;

	for (GameObject* pChild : m_vChildren)
	{
		if (pChild && !pChild->m_bIsDead)
		{
			pChild->Destroy();
		}
	}

	if (m_pOwnerScene)
	{
		m_pOwnerScene->DestroyObjects(this);
	}
}

void GameObject::SetInstanceID(uint64 id)
{
	m_instanceID = id;
	if (id >= s_nextInstanceID)
	{
		s_nextInstanceID = id + 1;
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
 
void GameObject::Serialize(json& outJson) const
{
	outJson[EngineKey::Property::Name.data()] = m_name;
	outJson["InstanceID"] = m_instanceID;
	outJson["ParentInstanceID"] = (m_pParent != nullptr) ? m_pParent->GetInstanceID() : 0;
	outJson[EngineKey::Property::IsActive.data()] = m_bIsActive;
	outJson[EngineKey::Property::Components.data()] = std::vector<json>();

	json transformJson;
	std::string trName = EngineKey::Component::Trnasform.data();

	transformJson[EngineKey::Property::Type.data()] = trName;

	json transformData;
	transform.Serialize(transformData);
	transformJson[EngineKey::Property::Data.data()] = transformData;

	outJson[EngineKey::Property::Components.data()].push_back(transformJson);

	for (auto* comp : m_vComponents)
	{
		if (!comp) continue;

		json compJson;
		compJson[EngineKey::Property::Type.data()] = comp->GetComponentType().data();

		json compData;
		comp->Serialize(compData);
		compJson[EngineKey::Property::Data.data()] = compData;

		outJson[EngineKey::Property::Components.data()].push_back(compJson);
	}
}

void GameObject::Deserialize(const json& inJson)
{
	if (inJson.contains(EngineKey::Property::Name.data()))
	{
		m_name = inJson[EngineKey::Property::Name.data()].get<std::string>();
	}

	if (inJson.contains("InstanceID"))
	{
		SetInstanceID(inJson["InstanceID"].get<uint64>());
	}

	if (inJson.contains("ParentInstanceID"))
	{
		m_parentInstanceID = inJson["ParentInstanceID"].get<uint64>();
	}

	if (inJson.contains(EngineKey::Property::IsActive.data()))
	{
		m_bIsActive = inJson[EngineKey::Property::IsActive.data()].get<bool>();
	}
}

void GameObject::PostDeserialize(Scene* pScene)
{
	if (m_parentInstanceID != 0 && pScene != nullptr)
	{
		GameObject* pParentObj = pScene->FindGameObjectByInstanceID(m_parentInstanceID);
		if (pParentObj != nullptr)
		{
			SetParent(pParentObj, false);
		}
	}

	transform.PostDeserialize(pScene);
	for (auto* comp : m_vComponents)
	{
		if (comp != nullptr && comp != m_pTransform)
		{
			comp->PostDeserialize(pScene);
		}
	}
}

void GameObject::SetSiblingIndex(int index)
{
	if (m_pOwnerScene)
	{
		m_pOwnerScene->ReorderGameObject(this, index);
	}
}

int GameObject::GetSiblingIndex() const
{
	return static_cast<int>(m_sceneIndex);
}

void GameObject::SetAsFirstSibling()
{
	SetSiblingIndex(0);
}

void GameObject::SetAsLastSibling()
{
	if (m_pOwnerScene)
	{
		SetSiblingIndex(static_cast<int>(m_pOwnerScene->GetGameObjects().size()) - 1);
	}
}

void GameObject::SetActive(bool active)
{
	if (m_bIsActive == active) return;

	m_bIsActive = active;

	for (auto* comp : m_vComponents)
	{
		if (!comp) continue;

		if (m_bIsActive)
		{
			if (!comp->HasAwoken())
			{
				comp->Awake();
				comp->MarkAwoken();
			}

			if (comp->IsEnabled())
			{
				comp->OnEnable();

				if (!comp->HasStarted())
				{
					comp->Start();
					comp->MarkStarted();
				}
			}
		}
		else
		{
			if (comp->IsEnabled())
			{
				comp->OnDisable();
			}
		}
	}

	for (GameObject* pChild : m_vChildren)
	{
		if (pChild) pChild->OnHierarchyActiveChanged(active);
	}
}

void GameObject::OnHierarchyActiveChanged(bool parentActive)
{
	bool wasActiveInHierarchy = m_bIsActive; 
	bool isNowActiveInHierarchy = m_bIsActive && parentActive;

	if (wasActiveInHierarchy != isNowActiveInHierarchy)
	{
		for (auto* comp : m_vComponents)
		{
			if (!comp || !comp->IsEnabled()) continue;
			if (isNowActiveInHierarchy)
				comp->OnEnable();
			else
				comp->OnDisable();
		}
	}

	for (GameObject* pChild : m_vChildren)
	{
		if (pChild) pChild->OnHierarchyActiveChanged(isNowActiveInHierarchy);
	}
}

bool GameObject::IsActiveInHierarchy() const
{
	if (!m_bIsActive) return false;
	if (m_pParent != nullptr) return m_pParent->IsActiveInHierarchy();
	return true;
}


bool GameObject::IsDescendantOf(const GameObject* potentialAncestor) const
{
	if (potentialAncestor == nullptr) return false;
	const GameObject* curr = m_pParent;
	while (curr != nullptr)
	{
		if (curr == potentialAncestor) return true;
		curr = curr->m_pParent;
	}
	return false;
}

void GameObject::SetParent(GameObject* pNewParent, bool keepWorldTransform)
{
	if (pNewParent == this) return;
	if (pNewParent == m_pParent) return;
	if (IsDescendantOf(pNewParent)) return; 

	Vector2 worldPosBefore = { 0.0f, 0.0f };
	float   worldRotBefore = 0.0f;
	Vector2 worldScaleBefore = { 1.0f, 1.0f };

	if (keepWorldTransform)
	{
		worldPosBefore    = m_pTransform->GetWorldPosition();
		worldRotBefore    = m_pTransform->GetWorldRotation();
		worldScaleBefore  = m_pTransform->GetWorldScale();
	}

	if (m_pParent != nullptr)
	{
		auto& siblings = m_pParent->m_vChildren;
		siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
		m_pTransform->DetachFromParent();
		m_pParent = nullptr;
	}

	m_pParent = pNewParent;
	if (m_pParent != nullptr)
	{
		m_pParent->m_vChildren.push_back(this);
		m_pTransform->AttachToParent(m_pParent->m_pTransform);
	}

	if (keepWorldTransform && m_pTransform)
	{
		if (m_pParent != nullptr)
		{
			Vector2 localPos = m_pTransform->WorldToLocal(worldPosBefore);
			m_pTransform->SetLocalPosition(localPos);
			
			float parentWorldRot = m_pParent->m_pTransform->GetWorldRotation();
			m_pTransform->SetLocalRotation(worldRotBefore - parentWorldRot);
		}
		else
		{
			m_pTransform->SetLocalPosition(worldPosBefore);
			m_pTransform->SetLocalRotation(worldRotBefore);
		}
	}

	if (m_pOwnerScene)
	{
		m_pOwnerScene->UpdateGameObjectIndices();
	}
}

void GameObject::RegisterObserverPtr(void** pObserverPtr)
{
	if (pObserverPtr && std::find(m_vObservers.begin(), m_vObservers.end(), pObserverPtr) == m_vObservers.end())
	{
		m_vObservers.push_back(pObserverPtr);
	}
}

void GameObject::UnregisterObserverPtr(void** pObserverPtr)
{
	auto it = std::find(m_vObservers.begin(), m_vObservers.end(), pObserverPtr);
	if (it != m_vObservers.end())
	{
		m_vObservers.erase(it);
	}
}
