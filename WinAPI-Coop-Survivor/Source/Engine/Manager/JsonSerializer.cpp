#include "Engine/Core/pch.h"
#include "JsonSerializer.h"
#include "FileSystem.h"
#include "Engine/Manager/PrefabManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Base/Component.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"

bool JsonSerializer::SaveScene(Scene* pScene, const std::string& filePath)
{
	if (pScene == nullptr) return false;

	json sceneJson;
	sceneJson[EngineKey::Document::SceneName.data()] = pScene->GetSceneName();
	sceneJson[EngineKey::Document::GameObjects.data()] = std::vector<json>();

	const auto& gameObjects = pScene->GetGameObjects();
	for (auto* obj : gameObjects)
	{
		if (obj == nullptr || obj->IsDead()) continue;
		sceneJson[EngineKey::Document::GameObjects.data()].push_back(SerializeGameObject(obj));
	}

	return FileSystem::WriteJson(filePath, sceneJson);
}


bool JsonSerializer::LoadScene(Scene* pScene, json& sceneJson)
{
	if (pScene == nullptr) return false;

	// 1-Pass: 모든 GameObject 생성
	if (sceneJson.contains(EngineKey::Document::GameObjects.data()))
	{
		for (const auto& objJson : sceneJson[EngineKey::Document::GameObjects.data()])
		{
			GameObject* newObj = pScene->CreateGameObject();
			ApplyJsonToGameObject(newObj, objJson);
		}
	}

	// 2-Pass: PostDeserialize에서 ParentInstanceID 기반 SetParent 바인딩
	pScene->PostDeserialize();

	return true;
}


bool JsonSerializer::SavePrefab(GameObject* pObj, const std::string& filePath)
{
	if (pObj == nullptr) return false;
	bool success = FileSystem::WriteJson(filePath, SerializeGameObject(pObj, true));
	if (success)
	{
		std::filesystem::path p(filePath);
		PrefabManager::GetInstance()->LoadPrefab(p.stem().string(), filePath);
	}
	return success;
}

GameObject* JsonSerializer::InstantiateFromPrefabData(Scene* pScene, const json& prefabJson)
{
	if (pScene == nullptr || prefabJson.empty()) return nullptr;

	GameObject* rootObj = pScene->CreateGameObject();
	ApplyJsonToGameObject(rootObj, prefabJson);

	rootObj->PostDeserialize(pScene);

	return rootObj;
}

GameObject* JsonSerializer::InstantiateHierarchyFromPrefab(Scene* pScene, const json& prefabJson)
{
	return InstantiateFromPrefabData(pScene, prefabJson);
}

json JsonSerializer::SerializeGameObject(GameObject* pObj, bool recursive)
{
	json objJson;
	if (pObj != nullptr)
	{
		pObj->Serialize(objJson);

		if (recursive && !pObj->GetChildren().empty())
		{
			objJson["Children"] = std::vector<json>();
			for (auto* pChild : pObj->GetChildren())
			{
				if (pChild != nullptr && !pChild->IsDead())
				{
					objJson["Children"].push_back(SerializeGameObject(pChild, true));
				}
			}
		}
	}
	return objJson;
}

void JsonSerializer::ApplyJsonToGameObject(GameObject* pObj, const json& objJson)
{
	if (pObj == nullptr || objJson.empty()) return;

	pObj->Deserialize(objJson);

	if (objJson.contains(EngineKey::Property::Components.data()))
	{
		for (const auto& compJson : objJson[EngineKey::Property::Components.data()])
		{
			if (!compJson.contains(EngineKey::Property::Type.data()) || !compJson.contains(EngineKey::Property::Data.data()))
			{
				continue;
			}

			std::string type = compJson[EngineKey::Property::Type.data()].get<std::string>();
			json data = compJson[EngineKey::Property::Data.data()];

			if (type == EngineKey::Component::Trnasform.data())
			{
				pObj->transform.Deserialize(data);
				continue;
			}

			auto it = GetComponentFactory().find(type);
			if (it != GetComponentFactory().end())
			{
				Component* newComp = it->second(pObj);
				if (newComp)
				{
					newComp->Deserialize(data);
				}
			}
		}
	}

	if (objJson.contains("Children"))
	{
		for (const auto& childJson : objJson["Children"])
		{
			GameObject* pChild = (pObj->GetOwnerScene() != nullptr)
				? pObj->GetOwnerScene()->CreateGameObject()
				: new GameObject(nullptr);

			ApplyJsonToGameObject(pChild, childJson);
			pChild->SetParent(pObj, false);
		}
	}
}

json JsonSerializer::SerializeScene(Scene* pScene)
{
	if (pScene == nullptr) return json();
	json sceneJson;
	sceneJson[EngineKey::Document::SceneName.data()] = pScene->GetSceneName();
	sceneJson[EngineKey::Document::GameObjects.data()] = std::vector<json>();
	const auto& gameObjects = pScene->GetGameObjects();
	for (auto* obj : gameObjects)
	{
		if (obj == nullptr || obj->IsDead()) continue;
		sceneJson[EngineKey::Document::GameObjects.data()].push_back(SerializeGameObject(obj));
	}
	return sceneJson;
}

void JsonSerializer::RegisterComponentFactory(const std::string& typeName, std::function<Component* (GameObject*)> factory)
{
	GetComponentFactory()[typeName] = factory;
}

std::unordered_map<std::string, std::function<Component* (GameObject*)>>& JsonSerializer::GetComponentFactory()
{
	static std::unordered_map<std::string, std::function<Component* (GameObject*)>> factory;
	return factory;
}
