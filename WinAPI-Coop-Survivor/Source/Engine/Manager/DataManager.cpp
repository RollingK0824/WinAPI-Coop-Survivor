#include "Engine/Core/pch.h"
#include "DataManager.h"
#include "FileSystem.h"
#include "Game/Monster/MonsterSO.h"
#include "Game/Skill/SkillSO.h"

bool DataManager::Initialize()
{
	LoadAllAssets("Resources");
	return true;
}

void DataManager::Release()
{
	m_assetTable.clear();
	GetSOFactories().clear();
}

void DataManager::RegisterSOFactory(const std::string& typeName, SOFactory factory)
{
	GetSOFactories()[typeName] = factory;
}

bool DataManager::LoadAllAssets(const std::string& directoryPath)
{
	const auto files = FileSystem::GetFilesInDirectory(directoryPath, ".asset", true);
	for (const auto& filePath : files)
	{
		LoadAssetFile(filePath);
	}
	return true;
}

bool DataManager::LoadAssetFile(const std::string& filePath)
{
	json j;
	if (!FileSystem::ReadJson(filePath, j)) return false;

	try
	{
		std::string typeStr = j.contains("Type") ? j["Type"].get<std::string>() : "MonsterSO";

		std::shared_ptr<ScriptableObject> pSO = nullptr;
		auto& factories = GetSOFactories();
		auto it = factories.find(typeStr);
		if (it != factories.end())
		{
			pSO = it->second();
		}

		if (pSO)
		{
			pSO->OnLoadFromJson(j);
			pSO->SetFilePath(filePath);
			if (pSO->GetAssetID() != 0)
			{
				m_assetTable[pSO->GetAssetID()] = pSO;
			}
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

bool DataManager::SaveAssetFile(ScriptableObject* pSO)
{
	if (!pSO) return false;
	std::string path = pSO->GetFilePath();
	if (path.empty())
	{
		path = "Resources/Data/" + pSO->GetAssetName() + ".asset";
		pSO->SetFilePath(path);
	}

	json j = pSO->SaveToJson();
	return FileSystem::WriteJson(path, j);
}

bool DataManager::LoadMonsterTable(const std::string& filePath)
{
	json dataJson;
	if (!FileSystem::ReadJson(filePath, dataJson))
	{
		return false;
	}

	try
	{
		if (dataJson.contains("Monsters") && dataJson["Monsters"].is_array())
		{
			for (const auto& itemJson : dataJson["Monsters"])
			{
				auto monsterSO = std::make_shared<MonsterSO>();
				monsterSO->OnLoadFromJson(itemJson);

				if (monsterSO->GetAssetID() != 0)
				{
					m_assetTable[monsterSO->GetAssetID()] = monsterSO;
				}
			}
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool DataManager::SaveMonsterTable(const std::string& filePath)
{
	json rootJson;
	json monstersArray = json::array();

	for (const auto& [id, pAsset] : m_assetTable)
	{
		if (pAsset && pAsset->GetSOTypeName() == "MonsterSO")
		{
			monstersArray.push_back(pAsset->SaveToJson());
		}
	}

	rootJson["Monsters"] = monstersArray;
	return FileSystem::WriteJson(filePath, rootJson);
}

std::shared_ptr<const MonsterSO> DataManager::GetMonsterSO(uint32 assetID) const
{
	return GetAsset<MonsterSO>(assetID);
}

std::shared_ptr<MonsterSO> DataManager::GetMutableMonsterSO(uint32 assetID)
{
	return GetMutableAsset<MonsterSO>(assetID);
}

std::shared_ptr<const SkillSO> DataManager::GetSkillSO(uint32 assetID) const
{
	return GetAsset<SkillSO>(assetID);
}

std::shared_ptr<SkillSO> DataManager::GetMutableSkillSO(uint32 assetID)
{
	return GetMutableAsset<SkillSO>(assetID);
}

bool DataManager::RemoveSO(uint32 assetID)
{
	auto it = m_assetTable.find(assetID);
	if (it != m_assetTable.end())
	{
		FileSystem::RemoveFile(it->second->GetFilePath());
		m_assetTable.erase(it);
		return true;
	}
	return false;
}
