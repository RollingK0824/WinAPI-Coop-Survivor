#include "Engine/Core/pch.h"
#include "PrefabManager.h"
#include "FileSystem.h"
#include "Engine/Manager/JsonSerializer.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Scene.h"

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
	const auto files = FileSystem::GetFilesInDirectory(directoryPath, ".prefab");
	for (const auto& filePath : files)
	{
		std::filesystem::path p(filePath);
		LoadPrefab(p.stem().string(), filePath);
	}
	return true;
}

bool PrefabManager::LoadPrefab(const std::string& key, const std::string& filePath)
{
	json prefabData;
	if (!FileSystem::ReadJson(filePath, prefabData)) return false;

	auto it = m_PrefabTemplates.find(key);
	if (it != m_PrefabTemplates.end() && it->second != nullptr)
	{
		delete it->second;
		m_PrefabTemplates.erase(it);
	}

	GameObject* templateObj = new GameObject(nullptr);
	JsonSerializer::ApplyJsonToGameObject(templateObj, prefabData);
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
