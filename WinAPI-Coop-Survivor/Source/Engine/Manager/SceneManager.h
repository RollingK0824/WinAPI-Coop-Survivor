#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IUpdatable.h"
#include "Engine/Framework/Base/IRenderable.h"

class Scene;

class SceneManager : public Singleton<SceneManager>, public ISystem, public IUpdatable, public IRenderable
{
	friend class Singleton<SceneManager>;
public:
	virtual bool Initialize()override;
	virtual void Release()override;

	virtual void FixedUpdate(float fixedDt)override;
	virtual void Update(float dt)override;
	virtual void LateUpdate(float dt)override;

	virtual void Render()override;

	virtual void PostFrame()override;

	bool CreateScene(const std::string& sceneName);
	bool LoadScene(const std::string& sceneName);
	Scene* GetActiveScene() const { return m_pActiveScene; }

	bool SaveActiveScene(const std::string& jsonFilePath = "Resources/Json/DefaultScene.json");
	bool LoadSceneFromFile(const std::string& jsonFilePath);

	void SavePlaySnapshot();
	void RestorePlaySnapshot();

private:
	std::unordered_map<std::string, Scene*> m_mapScenes;
	Scene* m_pActiveScene = nullptr;
	Scene* m_pNextScene = nullptr;

	json m_playSceneSnapshot;
	bool m_bHasSnapshot = false;

	void ChangeSceneInternal();
};