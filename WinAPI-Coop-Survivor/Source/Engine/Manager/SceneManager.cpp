#include "Engine/Core/pch.h"
#include "SceneManager.h"
#include "Engine/Core/EngineKernel.h"
#include "Engine/Manager/JsonSerializer.h"
#include "Engine/Editor/EditorSystem.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"

bool SceneManager::Initialize()
{
    // TODO : Initialize()

    if (!CreateScene(INTRO_SCENE))return false;

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
    m_pActiveScene->PostFrame();
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

    m_mapScenes[sceneName] = newScene;

    m_pActiveScene = newScene;

    return true;
}

bool SceneManager::LoadScene(const std::string& sceneName)
{
    auto it = m_mapScenes.find(sceneName);
    if (it == m_mapScenes.end())
    {
        return false;
    }

    m_pNextScene = it->second;
    return true;
}

bool SceneManager::SaveActiveScene(const std::string& jsonFilePath)
{
    if (m_pActiveScene == nullptr) return false;
    // 활성화된 씬의 이름을 DefaultScene으로 세팅 후 JsonSerializer를 이용해 저장
    m_pActiveScene->SetSceneName("DefaultScene");
    return JsonSerializer::SaveScene(m_pActiveScene, jsonFilePath);
}

bool SceneManager::LoadSceneFromFile(const std::string& jsonFilePath)
{
    std::ifstream file(jsonFilePath);
    if (!file.is_open()) return false;

    json sceneJson;
    file >> sceneJson;
    file.close();

    if (!sceneJson.contains(EngineKey::Document::SceneName.data())) return false;
    std::string sceneName = sceneJson[EngineKey::Document::SceneName.data()].get<std::string>();

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
    // 1. 현재 활성화된 m_pActiveScene을 RAM 메모리 json으로 백업
    m_playSceneSnapshot = JsonSerializer::SerializeScene(m_pActiveScene);
    m_bHasSnapshot = true;
}

void SceneManager::RestorePlaySnapshot()
{
    if (!m_pActiveScene || !m_bHasSnapshot) return;
    // 1. 활성 씬의 기존 오브젝트 해제 후 재초기화
    m_pActiveScene->Release();
    m_pActiveScene->Initialize();
    // 2. JsonSerializer를 사용하여 RAM 스냅샷 json에서 활성 씬 복원
    JsonSerializer::LoadScene(m_pActiveScene, m_playSceneSnapshot);
    // 3. 임시 스냅샷 메모리 비우기
    m_playSceneSnapshot.clear();
    m_bHasSnapshot = false;
}

void SceneManager::ChangeSceneInternal()
{
    if (m_pNextScene == nullptr)return;

    EditorSystem::GetInstance()->SetSelectedObject(nullptr);

    if(m_pActiveScene != nullptr && m_pActiveScene != m_pNextScene)m_pActiveScene->Release();

    m_pActiveScene = m_pNextScene;
    m_pNextScene = nullptr;
}
