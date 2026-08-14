#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/ScriptableObject.h"
#include "Engine/Manager/FileSystem.h"
#include <functional>
#include <memory>
#include <unordered_map>
#include <string>

class MonsterSO;
class SkillSO;

class DataManager : public Singleton<DataManager>, public ISystem
{
	friend class Singleton<DataManager>;

public:
	using SOFactory = std::function<std::shared_ptr<ScriptableObject>()>;

	virtual bool Initialize() override;
	virtual void Release() override;

	static std::unordered_map<std::string, SOFactory>& GetSOFactories()
	{
		static std::unordered_map<std::string, SOFactory> factories;
		return factories;
	}

	void RegisterSOFactory(const std::string& typeName, SOFactory factory);

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

	template<typename T>
	std::shared_ptr<T> CreateAsset(const std::string& name = "", const std::string& folderPath = "Resources/Data", uint32 baseID = 101)
	{
		uint32 newID = baseID;
		while (m_assetTable.find(newID) != m_assetTable.end())
		{
			newID++;
		}

		const std::string dirPath = folderPath.empty() ? "Resources/Data" : folderPath;
		FileSystem::CreateDirectoryPath(dirPath);

		auto pSO = std::make_shared<T>();
		pSO->SetAssetID(newID);
		std::string assetName = name.empty() ? "NewAsset_" + std::to_string(newID) : name;
		pSO->SetAssetName(assetName);
		pSO->SetFilePath(dirPath + "/" + assetName + ".asset");

		m_assetTable[newID] = pSO;
		SaveAssetFile(pSO.get());
		return pSO;
	}

	// Helper getters with forward declarations
	std::shared_ptr<const MonsterSO> GetMonsterSO(uint32 assetID) const;
	std::shared_ptr<MonsterSO> GetMutableMonsterSO(uint32 assetID);

	std::shared_ptr<const SkillSO> GetSkillSO(uint32 assetID) const;
	std::shared_ptr<SkillSO> GetMutableSkillSO(uint32 assetID);

	bool RemoveSO(uint32 assetID);

	const std::unordered_map<uint32, std::shared_ptr<ScriptableObject>>& GetAllAssets() const { return m_assetTable; }

private:
	DataManager() = default;
	virtual ~DataManager() = default;

private:
	std::unordered_map<uint32, std::shared_ptr<ScriptableObject>> m_assetTable;
};
