#include "Engine/Core/pch.h"
#include "DataManager.h"
#include "Game/Data/MonsterSO.h"
#include <fstream>
#include <iostream>

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
	if (!std::filesystem::exists(directoryPath)) return false;

	for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".asset")
		{
			LoadAssetFile(entry.path().string());
		}
	}
	return true;
}

bool DataManager::LoadAssetFile(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open()) return false;

	try
	{
		json j;
		file >> j;
		std::string typeStr = j.contains("Type") ? j["Type"].get<std::string>() : "MonsterSO";

		std::shared_ptr<ScriptableObject> pSO = nullptr;
		if (typeStr == "MonsterSO")
		{
			pSO = std::make_shared<MonsterSO>();
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
		std::filesystem::create_directories("Resources/Data");
		path = "Resources/Data/" + pSO->GetAssetName() + ".asset";
		pSO->SetFilePath(path);
	}

	std::ofstream file(path);
	if (!file.is_open()) return false;

	json j = pSO->SaveToJson();
	j["Type"] = "MonsterSO";

	file << j.dump(4);
	return true;
}

bool DataManager::LoadMonsterTable(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		std::cout << "[DataManager] Failed to open file: " << filePath << std::endl;
		return false;
	}

	try
	{
		json dataJson;
		file >> dataJson;

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
	std::ofstream file(filePath);
	if (!file.is_open())
	{
		std::cout << "[DataManager] Failed to open file for saving: " << filePath << std::endl;
		return false;
	}

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

	file << rootJson.dump(4);
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

std::shared_ptr<MonsterSO> DataManager::CreateMonsterSO(const std::string& name, const std::string& folderPath)
{
	uint32 newID = 101;
	while (m_assetTable.find(newID) != m_assetTable.end())
	{
		newID++;
	}

	std::filesystem::path dirPath = folderPath.empty() ? "Resources/Data" : folderPath;
	std::filesystem::create_directories(dirPath);

	auto monsterSO = std::make_shared<MonsterSO>();
	monsterSO->SetAssetID(newID);
	std::string assetName = name.empty() ? "NewMonster_" + std::to_string(newID) : name;
	monsterSO->SetAssetName(assetName);

	std::string fullPath = (dirPath / (assetName + ".asset")).string();
	monsterSO->SetFilePath(fullPath);

	m_assetTable[newID] = monsterSO;
	SaveAssetFile(monsterSO.get());
	return monsterSO;
}

bool DataManager::RemoveSO(uint32 assetID)
{
	auto it = m_assetTable.find(assetID);
	if (it != m_assetTable.end())
	{
		std::string filePath = it->second->GetFilePath();
		if (!filePath.empty() && std::filesystem::exists(filePath))
		{
			std::filesystem::remove(filePath);
		}
		m_assetTable.erase(it);
		return true;
	}
	return false;
}
