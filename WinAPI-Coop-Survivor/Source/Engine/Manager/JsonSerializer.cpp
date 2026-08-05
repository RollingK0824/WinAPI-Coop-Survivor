#include "Engine/Core/pch.h"
#include "JsonSerializer.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Base/Component.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"

bool JsonSerializer::SaveScene(Scene* pScene, const std::string& filePath)
{
	if (pScene == nullptr)return false;

	std::filesystem::path path(filePath);
	if (path.has_parent_path())
	{
		std::filesystem::create_directories(path.parent_path());
	}

	json sceneJson;
	sceneJson[EngineKey::Document::SceneName.data()] = pScene->GetSceneName();
	sceneJson[EngineKey::Document::GameObjects.data()] = std::vector<json>();

	const auto& gameObjects = pScene->GetGameObjects();
	for (auto* obj : gameObjects)
	{
		if (obj == nullptr || obj->IsDead())continue;

		sceneJson[EngineKey::Document::GameObjects.data()].push_back(SerializeGameObject(obj));
	}

	std::ofstream file(filePath);
	if (!file.is_open())
	{
		std::cout << "파일을 저장할 수 없습니다. ->" << filePath << std::endl;
		return false;
	}

	file << sceneJson.dump(4);
	file.close();
	return true;
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
	return true;
}


bool JsonSerializer::SavePrefab(GameObject* pObj, const std::string& filePath)
{
	if (pObj == nullptr) return false;
	// SaveScene과 100% 동일한 경로 및 디렉터리 생성 로직
	std::filesystem::path finalPath(filePath);
	if (finalPath.has_parent_path())
	{
		std::filesystem::create_directories(finalPath.parent_path());
	}
	json prefabJson = SerializeGameObject(pObj);
	std::ofstream file(finalPath);
	if (!file.is_open()) return false;
	file << prefabJson.dump(4);
	file.close();
	return true;
}

GameObject* JsonSerializer::InstantiateFromPrefabData(Scene* pScene, const json& prefabJson)
{
	if (pScene == nullptr || prefabJson.empty()) return nullptr;

	GameObject* cloneObj = pScene->CreateGameObject();

	ApplyJsonToGameObject(cloneObj, prefabJson);

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
