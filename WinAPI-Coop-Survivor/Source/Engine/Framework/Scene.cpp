#include "Engine/Core/pch.h"
#include "Scene.h"
#include "Engine/Core/EngineKernel.h"
#include "Engine/Manager/JsonSerializer.h"
#include "Engine/Renderer/RenderSystem.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Base/Component.h"
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Framework/Components/Core/RenderComponent.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"

Scene::~Scene()
{
	Release();
}

bool Scene::Initialize()
{
	m_vGameObjects.reserve(128);
	m_vUpdatableComponents.reserve(128);
	m_vRenderComponents.reserve(128);
	return true;
}

void Scene::Release()
{
	for (auto* obj : m_vGameObjects)
	{
		if (obj != nullptr) delete obj;
	}

	m_vGameObjects.clear();
	m_vUpdatableComponents.clear();
	m_vRenderComponents.clear();

	std::vector<GameObject*>().swap(m_vGameObjects);
	std::vector<ScriptComponent*>().swap(m_vUpdatableComponents);
	std::vector<RenderComponent*>().swap(m_vRenderComponents);
}

void Scene::FixedUpdate(float fixedDt)
{
	for (auto* updatable : m_vUpdatableComponents)
	{
		if (updatable && updatable->IsEnabled() && updatable->gameObject.IsActive())
		{
			updatable->FixedUpdate(fixedDt);
		}
	}
}

void Scene::Update(float dt)
{
	for (auto* updatable : m_vUpdatableComponents)
	{
		if (updatable && updatable->IsEnabled() && updatable->gameObject.IsActive())
		{
			updatable->Update(dt);
		}
	}
}

void Scene::LateUpdate(float dt)
{
	for (auto* updatable : m_vUpdatableComponents)
	{
		if (updatable && updatable->IsEnabled() && updatable->gameObject.IsActive())
		{
			updatable->LateUpdate(dt);
		}
	}
}

void Scene::Render()
{
	for (auto* renderComp : m_vRenderComponents)
	{
		if (renderComp == nullptr || !renderComp->IsEnabled() || !renderComp->gameObject.IsActive()) continue;

		RenderCommand cmd = renderComp->GetRenderCommand();

		TransformComponent* transform = &renderComp->transform;
		if (transform)
		{
			cmd.position = transform->GetPosition();
			cmd.rotation = transform->GetRotation().angle;
			cmd.scaleX = transform->GetScale().x;
			cmd.scaleY = transform->GetScale().y;
		}

		RenderSystem::GetInstance()->SubmitCommand(cmd);
	}
}

bool Scene::Save(const std::string& filePath)
{
	json outJson;
	outJson[EngineKey::Document::SceneName.data()] = m_SceneName;
	return true;
}

bool Scene::Load(const std::string& filePath)
{
	json inJson;

	if (inJson.contains(EngineKey::Document::SceneName.data()))
	{
		m_SceneName = inJson[EngineKey::Document::SceneName.data()].get<std::string>();
	}
	return true;
}

GameObject* Scene::CreateGameObject()
{
	GameObject* newObj = new GameObject(this);
	//newObj->SetSceneIndex(m_vGameObjects.size());
	//m_vGameObjects.push_back(newObj);
	m_vCreationQueue.push_back(newObj);
	return newObj;
}

GameObject* Scene::CreateGameObject(const std::string& objName)
{
	GameObject* newObj = new GameObject(this);
	/*newObj->SetSceneIndex(m_vGameObjects.size());
	newObj->SetName(objName);
	m_vGameObjects.push_back(newObj);*/
	newObj->SetName(objName);
	m_vCreationQueue.push_back(newObj);
	return newObj;
}

void Scene::OnComponentAdded(Component* pComponent)
{
	if (pComponent == nullptr) return;

	pComponent->SetDestroyCallback([this](Component* targetComp)
		{
			if (auto* updatable = dynamic_cast<ScriptComponent*>(targetComp))
			{
				this->UnregisterScriptComponent(updatable);
			}

			if (auto* renderComp = dynamic_cast<RenderComponent*>(targetComp))
			{
				this->UnregisterRenderComponent(renderComp);
			}
		});

	m_vComponentCreationQueue.push_back(pComponent);
}

void Scene::UnregisterScriptComponent(ScriptComponent* pComp)
{
	if (m_vUpdatableComponents.empty() || pComp == nullptr) return;

	size_t deleteIdx = pComp->GetSceneVectorIndex();
	if (deleteIdx >= m_vUpdatableComponents.size() || m_vUpdatableComponents[deleteIdx] != pComp)
	{
		auto it = std::find(m_vUpdatableComponents.begin(), m_vUpdatableComponents.end(), pComp);
		if (it != m_vUpdatableComponents.end())
		{
			deleteIdx = std::distance(m_vUpdatableComponents.begin(), it);
		}
		else
		{
			return;
		}
	}

	size_t lastIdx = m_vUpdatableComponents.size() - 1;

	if (deleteIdx != lastIdx)
	{
		m_vUpdatableComponents[deleteIdx] = m_vUpdatableComponents[lastIdx];
		m_vUpdatableComponents[deleteIdx]->SetSceneVectorIndex(deleteIdx);
	}
	m_vUpdatableComponents.pop_back();
}

void Scene::UnregisterRenderComponent(RenderComponent* pComp)
{
	if (m_vRenderComponents.empty() || pComp == nullptr) return;

	size_t deleteIdx = pComp->GetSceneVectorIndex();
	if (deleteIdx >= m_vRenderComponents.size() || m_vRenderComponents[deleteIdx] != pComp)
	{
		auto it = std::find(m_vRenderComponents.begin(), m_vRenderComponents.end(), pComp);
		if (it != m_vRenderComponents.end())
		{
			deleteIdx = std::distance(m_vRenderComponents.begin(), it);
		}
		else
		{
			return;
		}
	}

	size_t lastIdx = m_vRenderComponents.size() - 1;

	if (deleteIdx != lastIdx)
	{
		m_vRenderComponents[deleteIdx] = m_vRenderComponents[lastIdx];
		m_vRenderComponents[deleteIdx]->SetSceneVectorIndex(deleteIdx);
	}
	m_vRenderComponents.pop_back();
}

void Scene::DestroyObjects(GameObject* pObj)
{
	if (pObj == nullptr)return;

	m_vDestroyQueue.push_back(pObj);
}

void Scene::PostFrameCleanUp()
{
	int iterationCount = 0;
	constexpr int MAX_CREATION_ITERATIONS = 100;

	while (!m_vCreationQueue.empty() || !m_vComponentCreationQueue.empty())
	{
		if (++iterationCount > MAX_CREATION_ITERATIONS)
		{
			break;
		}

		if (!m_vCreationQueue.empty())
		{
			std::vector<GameObject*> tempObjQueue;
			tempObjQueue.swap(m_vCreationQueue);

			for (GameObject* pObj : tempObjQueue)
			{
				if (pObj == nullptr) continue;
				pObj->SetSceneIndex(m_vGameObjects.size());
				m_vGameObjects.push_back(pObj);
			}
		}

		if (!m_vComponentCreationQueue.empty())
		{
			std::vector<Component*> tempCompQueue;
			tempCompQueue.swap(m_vComponentCreationQueue);

			for (Component* pComp : tempCompQueue)
			{
				if (pComp == nullptr) continue;

				if (EngineKernel::GetInstance()->GetPlayState() == EnginePlayState::Play)
				{
					pComp->Awake();
					pComp->Start();
					pComp->OnEnable();
				}

				if (auto* updatable = dynamic_cast<ScriptComponent*>(pComp))
				{
					pComp->SetSceneVectorIndex(m_vUpdatableComponents.size());
					m_vUpdatableComponents.push_back(updatable);
				}

				if (auto* renderComp = dynamic_cast<RenderComponent*>(pComp))
				{
					pComp->SetSceneVectorIndex(m_vRenderComponents.size());
					m_vRenderComponents.push_back(renderComp);
				}
			}
		}
	}

	if (!m_vDestroyQueue.empty())
	{
		for (GameObject* pObj : m_vDestroyQueue)
		{
			if (pObj == nullptr) continue;

			size_t targetIdx = pObj->GetSceneIndex();
			size_t lastIdx = m_vGameObjects.size() - 1;

			if (targetIdx != lastIdx)
			{
				m_vGameObjects[targetIdx] = m_vGameObjects[lastIdx];
				m_vGameObjects[targetIdx]->SetSceneIndex(targetIdx);
			}

			m_vGameObjects.pop_back();

			delete pObj;
		}

		m_vDestroyQueue.clear();
	}
}

void Scene::ReorderGameObject(GameObject* targetObj, int newIndex)
{
	if (!targetObj || newIndex < 0 || newIndex >= (int)m_vGameObjects.size()) return;
	auto it = std::find(m_vGameObjects.begin(), m_vGameObjects.end(), targetObj);
	if (it == m_vGameObjects.end()) return;
	m_vGameObjects.erase(it);
	m_vGameObjects.insert(m_vGameObjects.begin() + newIndex, targetObj);
	UpdateGameObjectIndices();
}

void Scene::UpdateGameObjectIndices()
{
	for (size_t i = 0; i < m_vGameObjects.size(); ++i)
	{
		if (m_vGameObjects[i])
		{
			m_vGameObjects[i]->SetSceneIndex(i);
		}
	}
}

GameObject* Scene::FindGameObjectByName(const std::string& name) const
{
	for (GameObject* pObj : m_vGameObjects)
	{
		if (pObj != nullptr && pObj->GetName() == name)
		{
			return pObj;
		}
	}
	return nullptr;
}

GameObject* Scene::FindGameObjectByInstanceID(uint64 instanceID) const
{
	for (GameObject* pObj : m_vGameObjects)
	{
		if (pObj != nullptr && pObj->GetInstanceID() == instanceID)
		{
			return pObj;
		}
	}
	for (GameObject* pObj : m_vCreationQueue)
	{
		if (pObj != nullptr && pObj->GetInstanceID() == instanceID)
		{
			return pObj;
		}
	}
	return nullptr;
}

void Scene::PostDeserialize()
{
	for (GameObject* pObj : m_vGameObjects)
	{
		if (pObj != nullptr)
		{
			pObj->PostDeserialize(this);
		}
	}
	for (GameObject* pObj : m_vCreationQueue)
	{
		if (pObj != nullptr)
		{
			pObj->PostDeserialize(this);
		}
	}
}

