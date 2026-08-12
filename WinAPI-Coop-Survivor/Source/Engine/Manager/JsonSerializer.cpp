#include "Engine/Core/pch.h"
#include "JsonSerializer.h"
#include "FileSystem.h"
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

	if (sceneJson.contains(EngineKey::Document::GameObjects.data()))
	{
		for (const auto& objJson : sceneJson[EngineKey::Document::GameObjects.data()])
		{
			GameObject* newObj = pScene->CreateGameObject();

			ApplyJsonToGameObject(newObj, objJson);
		}
	}

	pScene->PostDeserialize();

	return true;
}


bool JsonSerializer::SavePrefab(GameObject* pObj, const std::string& filePath)
{
	if (pObj == nullptr) return false;
	return FileSystem::WriteJson(filePath, SerializeGameObject(pObj));
}

GameObject* JsonSerializer::InstantiateFromPrefabData(Scene* pScene, const json& prefabJson)
{
	if (pScene == nullptr || prefabJson.empty()) return nullptr;

	GameObject* cloneObj = pScene->CreateGameObject();

	ApplyJsonToGameObject(cloneObj, prefabJson);

	cloneObj->PostDeserialize(pScene);

	return cloneObj;
}

json JsonSerializer::SerializeGameObject(GameObject* pObj)
{
	json objJson;
	if (pObj != nullptr)
	{
		pObj->Serialize(objJson);
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
