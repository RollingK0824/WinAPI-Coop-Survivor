#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"

class GameObject;
class Scene;

class PrefabManager : public Singleton<PrefabManager>, public ISystem
{
	friend class Singleton<PrefabManager>;
public:
	virtual bool Initialize() override;
	virtual void Release() override;

	bool LoadAllPrefabs(const std::string& directoryPath);

	bool LoadPrefab(const std::string& key, const std::string& filePath);

	// 미리 조립된 GameObject 원본(Template)을 깊은 복사(Deep Copy)하여 새 인스턴스로 반환합니다.
	GameObject* Instantiate(const std::string& prefabKey, Scene* pScene);
private:
	PrefabManager() = default;
	virtual ~PrefabManager() = default;

	std::unordered_map<std::string, GameObject*> m_PrefabTemplates;
};
