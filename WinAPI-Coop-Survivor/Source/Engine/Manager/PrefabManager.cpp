#include "Engine/Core/pch.h"
#include "PrefabManager.h"
#include "Engine/Manager/JsonSerializer.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Scene.h"

namespace fs = std::filesystem;

bool PrefabManager::Initialize()
{
	LoadAllPrefabs("Resources/Prefabs");
	return true;
}

void PrefabManager::Release()
{
	for (auto& pair : m_PrefabTemplates)
	{
		if (pair.second != nullptr)
		{
			delete pair.second;
		}
	}
	m_PrefabTemplates.clear();
}

bool PrefabManager::LoadAllPrefabs(const std::string& directoryPath)
{
	if (!fs::exists(directoryPath)) return false;

	for (const auto& entry : fs::directory_iterator(directoryPath))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".prefab")
		{
			std::string key = entry.path().stem().string();
			LoadPrefab(key, entry.path().string());
		}
	}
	return true;
}

bool PrefabManager::LoadPrefab(const std::string& key, const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open()) return false;

	json prefabData;
	file >> prefabData;
	file.close();

	GameObject* templateObj = new GameObject(nullptr);

	JsonSerializer::ApplyJsonToGameObject(templateObj, prefabData);

	templateObj->SetActive(false);

	m_PrefabTemplates[key] = templateObj;

	return true;
}

GameObject* PrefabManager::Instantiate(const std::string& prefabKey, Scene* pScene)
{
	auto it = m_PrefabTemplates.find(prefabKey);
	if (it != m_PrefabTemplates.end())
	{
		return it->second->Clone(pScene);
	}

	return nullptr;
}
