#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/ScriptableObject.h"
#include <unordered_map>
#include <memory>
#include <string>

class MonsterSO;

class DataManager : public Singleton<DataManager>, public ISystem
{
	friend class Singleton<DataManager>;

public:
	virtual bool Initialize() override;
	virtual void Release() override;

	bool LoadAllAssets(const std::string& directoryPath = "Resources");
	bool LoadAssetFile(const std::string& filePath);
	bool SaveAssetFile(ScriptableObject* pSO);

	bool LoadMonsterTable(const std::string& filePath);
	bool SaveMonsterTable(const std::string& filePath = "Resources/Json/Monsters.json");

	template<typename T>
	std::shared_ptr<const T> GetAsset(uint32 assetID) const
	{
		auto it = m_assetTable.find(assetID);
		if (it != m_assetTable.end())
		{
			return std::dynamic_pointer_cast<const T>(it->second);
		}
		return nullptr;
	}

	template<typename T>
	std::shared_ptr<T> GetMutableAsset(uint32 assetID)
	{
		auto it = m_assetTable.find(assetID);
		if (it != m_assetTable.end())
		{
			return std::dynamic_pointer_cast<T>(it->second);
		}
		return nullptr;
	}

	std::shared_ptr<const MonsterSO> GetMonsterSO(uint32 assetID) const;
	std::shared_ptr<MonsterSO> GetMutableMonsterSO(uint32 assetID);

	std::shared_ptr<MonsterSO> CreateMonsterSO(const std::string& name = "NewMonster", const std::string& folderPath = "Resources/Data");
	bool RemoveSO(uint32 assetID);

	const std::unordered_map<uint32, std::shared_ptr<ScriptableObject>>& GetAllAssets() const { return m_assetTable; }

private:
	DataManager() = default;
	virtual ~DataManager() = default;

private:
	std::unordered_map<uint32, std::shared_ptr<ScriptableObject>> m_assetTable;
};
