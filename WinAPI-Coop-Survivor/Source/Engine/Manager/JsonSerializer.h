#pragma once

class Scene;
class GameObject;
class Component;

class JsonSerializer
{
public:
	static bool SaveScene(Scene* pScene, const std::string& filePath);
	static bool LoadScene(Scene* pScene, json& sceneJson);

	static void RegisterComponentFactory(const std::string& typeName, std::function<Component* (GameObject*)> factory);

	static bool SavePrefab(GameObject* pObj, const std::string& saveDirectory);

	static GameObject* InstantiateFromPrefabData(Scene* pScene, const json& prefabJson);

	static std::vector<std::string> GetRegisteredComponentNames()
	{
		std::vector<std::string> names;
		for (const auto& it : GetComponentFactory())
		{
			names.push_back(it.first);
		}
		return names;
	}

	static Component* CreateComponent(const std::string& typeName, GameObject* pObj)
	{
		auto& factory = GetComponentFactory();
		auto it = factory.find(typeName);
		if (it != factory.end())
		{
			return it->second(pObj);
		}
		return nullptr;
	}
	
	static void ApplyJsonToGameObject(GameObject* pObj, const json& objJson);

	static json SerializeScene(Scene* pScene);

private:
	static json SerializeGameObject(GameObject* pObj);

	static std::unordered_map<std::string, std::function<Component* (GameObject*)>>& GetComponentFactory();
};