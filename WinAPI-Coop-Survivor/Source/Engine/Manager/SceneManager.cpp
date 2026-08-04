#include "Engine/Core/pch.h"
#include "SceneManager.h"
#include "Engine/Core/EngineKernel.h"
#include "Engine/Manager/JsonSerializer.h"
#include "Engine/Editor/EditorSystem.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/CameraComponent.h"

bool SceneManager::Initialize()
{
	// TODO : Initialize()
	return true;
}

void SceneManager::Release()
{
	for (auto& pair : m_mapScenes)
	{
		if (pair.second != nullptr)
		{
			pair.second->Release();
			delete pair.second;
		}
	}
	m_mapScenes.clear();

	m_pActiveScene = nullptr;
	m_pNextScene = nullptr;

	std::unordered_map<std::string, Scene*>().swap(m_mapScenes);
}

void SceneManager::FixedUpdate(float fixedDt)
{
	if (m_pNextScene != nullptr)
	{
		ChangeSceneInternal();
	}

	if (m_pActiveScene != nullptr)
	{
		m_pActiveScene->FixedUpdate(fixedDt);
	}
}
void SceneManager::Update(float dt)
{
	if (EngineKernel::GetInstance()->GetPlayState() != EnginePlayState::Play)return;

	if (m_pActiveScene != nullptr)
	{
		m_pActiveScene->Update(dt);
	}
}

void SceneManager::LateUpdate(float dt)
{
	if (m_pActiveScene != nullptr)
	{
		m_pActiveScene->LateUpdate(dt);
	}
}

void SceneManager::Render()
{
	if (m_pActiveScene != nullptr)
	{
		m_pActiveScene->Render();
	}
}

void SceneManager::PostFrame()
{
	if (m_pActiveScene != nullptr)
	{
		m_pActiveScene->PostFrame();
	}

	if (m_pNextScene != nullptr)
	{
		ChangeSceneInternal();
	}
}

void SceneManager::StartPlaySession()
{
	SavePlaySnapshot();
	EngineKernel::GetInstance()->SetPlayState(EnginePlayState::Play);
	RestorePlaySnapshot();
}

void SceneManager::StopPlaySession()
{
	EngineKernel::GetInstance()->SetPlayState(EnginePlayState::Edit);

	RestorePlaySnapshot();
}

bool SceneManager::CreateScene(const std::string& sceneName)
{
	if (m_mapScenes.find(sceneName) != m_mapScenes.end())
	{
		return false;
	}
	Scene* newScene = new Scene();
	if (!newScene->Initialize())
	{
		delete newScene;
		return false;
	}
	newScene->SetSceneName(sceneName);
	m_mapScenes[sceneName] = newScene;

	if (m_pActiveScene == nullptr)
	{
		m_pActiveScene = newScene;
	}

	return true;
}

Scene* SceneManager::CreateDefaultTemplateScene(const std::string& sceneName)
{
	if (!CreateScene(sceneName)) return nullptr;
	GameObject* pCameraObj = m_pActiveScene->CreateGameObject("Main Camera");
	pCameraObj->AddComponent<CameraComponent>();
	return m_pActiveScene;
}

bool SceneManager::LoadScene(const std::string& sceneName)
{
	auto it = m_mapScenes.find(sceneName);
	if (it == m_mapScenes.end())
	{
		return false;
	}

	if (m_pActiveScene == nullptr)
	{
		m_pActiveScene = it->second;
	}
	else
	{
		m_pNextScene = it->second;
	}

	return true;
}

bool SceneManager::SaveActiveScene(const std::string& jsonFilePath)
{
	if (!m_pActiveScene) return false;
	std::string targetPath = jsonFilePath;
	if (targetPath.empty() ||
		targetPath == "Resources/Json/DefaultScene.json" ||
		targetPath == "Resources/Scenes/DefaultScene.scene")
	{
		std::string sceneName = m_pActiveScene->GetSceneName();
		if (sceneName.empty()) sceneName = "DefaultScene";

		targetPath = "Resources/Scenes/" + sceneName + ".scene";
	}
	std::filesystem::path p(targetPath);
	m_pActiveScene->SetSceneName(p.stem().string());

	return JsonSerializer::SaveScene(m_pActiveScene, targetPath);
}

bool SceneManager::LoadSceneFromFile(const std::string& jsonFilePath)
{
	std::ifstream file(jsonFilePath);
	if (!file.is_open()) return false;
	json sceneJson;
	file >> sceneJson;
	file.close();

	std::filesystem::path p(jsonFilePath);
	std::string sceneName = p.stem().string();
	Scene* targetScene = nullptr;
	auto iter = m_mapScenes.find(sceneName);
	if (iter == m_mapScenes.end())
	{
		CreateScene(sceneName);
		targetScene = m_mapScenes[sceneName];
	}
	else
	{
		targetScene = iter->second;
		EditorSystem::GetInstance()->SetSelectedObject(nullptr);
		targetScene->Release();
		targetScene->Initialize();
	}

	targetScene->SetSceneName(sceneName);
	if (!JsonSerializer::LoadScene(targetScene, sceneJson))
	{
		return false;
	}
	return LoadScene(sceneName);
}

void SceneManager::SavePlaySnapshot()
{
	if (!m_pActiveScene) return;
	m_playSceneSnapshot.clear();
	m_playSceneSnapshot = JsonSerializer::SerializeScene(m_pActiveScene);
	m_bHasSnapshot = true;
}

void SceneManager::RestorePlaySnapshot()
{
	if (!m_pActiveScene || !m_bHasSnapshot) return;
	m_pActiveScene->Release();
	m_pActiveScene->Initialize();
	JsonSerializer::LoadScene(m_pActiveScene, m_playSceneSnapshot);
	m_playSceneSnapshot.clear();
	m_bHasSnapshot = false;
}

void SceneManager::ChangeSceneInternal()
{
	if (m_pNextScene == nullptr)return;

	EditorSystem::GetInstance()->SetSelectedObject(nullptr);

	if (m_pActiveScene != nullptr && m_pActiveScene != m_pNextScene)m_pActiveScene->Release();

	m_pActiveScene = m_pNextScene;
	m_pNextScene = nullptr;
}
