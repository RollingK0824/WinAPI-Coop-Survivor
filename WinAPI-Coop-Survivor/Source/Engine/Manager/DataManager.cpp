#include "Engine/Core/pch.h"
#include "DataManager.h"
#include "FileSystem.h"
#include "Game/Data/MonsterSO.h"
#include "Game/Skill/SkillSO.h"

bool DataManager::Initialize()
{
	LoadAllAssets("Resources");
	return true;
}

void DataManager::Release()
{
	m_assetTable.clear();
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
		if (typeStr == "SkillSO")
		{
			pSO = std::make_shared<SkillSO>();
		}
		else
		{
			pSO = std::make_shared<MonsterSO>();
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
	j["Type"] = dynamic_cast<SkillSO*>(pSO) ? "SkillSO" : "MonsterSO";

	return FileSystem::WriteJson(path, j);
}

bool DataManager::LoadMonsterTable(const std::string& filePath)
{
	json dataJson;
	if (!FileSystem::ReadJson(filePath, dataJson))
	{
		std::cout << "[DataManager] Failed to open file: " << filePath << std::endl;
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
					std::cout << "[DataManager] Loaded MonsterSO: ID=" << monsterSO->GetAssetID()
						<< ", Name=" << monsterSO->GetAssetName() << std::endl;
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "[DataManager] JSON Parsing Exception: " << e.what() << std::endl;
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
		if (auto monsterSO = std::dynamic_pointer_cast<MonsterSO>(pAsset))
		{
			monstersArray.push_back(monsterSO->SaveToJson());
		}
	}

	rootJson["Monsters"] = monstersArray;

	if (!FileSystem::WriteJson(filePath, rootJson))
	{
		std::cout << "[DataManager] Failed to open file for saving: " << filePath << std::endl;
		return false;
	}

	std::cout << "[DataManager] Successfully saved MonsterTable to: " << filePath << std::endl;
	return true;
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

std::shared_ptr<MonsterSO> DataManager::CreateMonsterSO(const std::string& name, const std::string& folderPath)
{
	uint32 newID = 101;
	while (m_assetTable.find(newID) != m_assetTable.end())
	{
		newID++;
	}

	const std::string dirPath = folderPath.empty() ? "Resources/Data" : folderPath;
	FileSystem::CreateDirectoryPath(dirPath);

	auto monsterSO = std::make_shared<MonsterSO>();
	monsterSO->SetAssetID(newID);
	std::string assetName = name.empty() ? "NewMonster_" + std::to_string(newID) : name;
	monsterSO->SetAssetName(assetName);
	monsterSO->SetFilePath(dirPath + "/" + assetName + ".asset");

	m_assetTable[newID] = monsterSO;
	SaveAssetFile(monsterSO.get());
	return monsterSO;
}

std::shared_ptr<SkillSO> DataManager::CreateSkillSO(const std::string& name, const std::string& folderPath)
{
	uint32 newID = 301;
	while (m_assetTable.find(newID) != m_assetTable.end())
	{
		newID++;
	}

	const std::string dirPath = folderPath.empty() ? "Resources/Data" : folderPath;
	FileSystem::CreateDirectoryPath(dirPath);

	auto skillSO = std::make_shared<SkillSO>();
	skillSO->SetAssetID(newID);
	std::string assetName = name.empty() ? "NewSkill_" + std::to_string(newID) : name;
	skillSO->SetAssetName(assetName);
	skillSO->SetFilePath(dirPath + "/" + assetName + ".asset");

	m_assetTable[newID] = skillSO;
	SaveAssetFile(skillSO.get());
	return skillSO;
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
