#include "Engine/Core/pch.h"
#include "ContentBrowserPanel.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Manager/JsonSerializer.h"
#include "Engine/Manager/PrefabManager.h"
#include "Engine/Manager/DataManager.h"
#include "Engine/Framework/Base/ScriptableObject.h"
#include "Game/Data/MonsterSO.h"
#include "Engine/Editor/EditorSystem.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"

void ContentBrowserPanel::Initialize()
{
    GUISystem::GetInstance()->RegisterPanel(this);
}

void ContentBrowserPanel::Release()
{
    GUISystem::GetInstance()->UnRegisterPanel(this);
}

void ContentBrowserPanel::OnDrawGUI()
{
    ImGui::Begin("Content Browser");
    DrawTopBar();
    DrawContentGrid();
    HandleWindowContextMenu();
    HandleRenameModal();
    ImGui::End();
}

void ContentBrowserPanel::DrawTopBar()
{
    if (m_CurrentDirectory != std::filesystem::path("Resources"))
    {
        if (ImGui::Button("<- Back"))
        {
            m_CurrentDirectory = m_CurrentDirectory.parent_path();
        }
        ImGui::Separator();
    }
}

void ContentBrowserPanel::DrawContentGrid()
{
    if (!std::filesystem::exists(m_CurrentDirectory)) return;
    for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
    {
        const auto& path = directoryEntry.path();
        std::string filenameStr = path.filename().string();
        if (directoryEntry.is_directory())
        {
            if (ImGui::Button(("[Folder] " + filenameStr).c_str()))
            {
                m_CurrentDirectory /= path.filename();
            }
        }
        else
        {
            bool isSceneFile = (path.extension() == ".scene");
            bool isPrefabFile = (path.extension() == ".prefab");
            bool isSOAssetFile = (path.extension() == ".asset");

            std::string prefix = isSOAssetFile ? "[SO Asset] " : "[File] ";
            bool isSelected = false;

            if (isSOAssetFile)
            {
                ScriptableObject* pSelectedSO = EditorSystem::GetInstance()->GetSelectedScriptableObject();
                if (pSelectedSO && pSelectedSO->GetFilePath() == path.string())
                {
                    isSelected = true;
                }
            }

            if (ImGui::Selectable((prefix + filenameStr).c_str(), isSelected))
            {
                if (isSOAssetFile)
                {
                    // 해당 .asset 파일의 SO 찾기
                    const auto& assets = DataManager::GetInstance()->GetAllAssets();
                    bool found = false;
                    for (const auto& [id, pAsset] : assets)
                    {
                        if (pAsset && pAsset->GetFilePath() == path.string())
                        {
                            EditorSystem::GetInstance()->SetSelectedScriptableObject(pAsset.get());
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        DataManager::GetInstance()->LoadAssetFile(path.string());
                        const auto& updatedAssets = DataManager::GetInstance()->GetAllAssets();
                        for (const auto& [id, pAsset] : updatedAssets)
                        {
                            if (pAsset && pAsset->GetFilePath() == path.string())
                            {
                                EditorSystem::GetInstance()->SetSelectedScriptableObject(pAsset.get());
                                break;
                            }
                        }
                    }
                }
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (isSceneFile)
                {
                    SceneManager::GetInstance()->LoadSceneFromFile(path.string());
                }
            }

            if (ImGui::BeginPopupContextItem())
            {
                if (isSOAssetFile && ImGui::MenuItem("Delete Asset"))
                {
                    const auto& assets = DataManager::GetInstance()->GetAllAssets();
                    uint32 toDeleteID = 0;
                    for (const auto& [id, pAsset] : assets)
                    {
                        if (pAsset && pAsset->GetFilePath() == path.string())
                        {
                            toDeleteID = id;
                            break;
                        }
                    }
                    if (toDeleteID != 0)
                    {
                        ScriptableObject* pSelectedSO = EditorSystem::GetInstance()->GetSelectedScriptableObject();
                        if (pSelectedSO && pSelectedSO->GetAssetID() == toDeleteID)
                        {
                            EditorSystem::GetInstance()->SetSelectedScriptableObject(nullptr);
                        }
                        DataManager::GetInstance()->RemoveSO(toDeleteID);
                    }
                    else if (std::filesystem::exists(path))
                    {
                        std::filesystem::remove(path);
                    }
                }

                if (isSceneFile && ImGui::MenuItem("Set as Active Scene"))
                {
                    SceneManager::GetInstance()->LoadSceneFromFile(path.string());
                }

                if (isPrefabFile && ImGui::MenuItem("Instantiate Prefab"))
                {
                    Scene* pActiveScene = SceneManager::GetInstance()->GetActiveScene();
                    if (pActiveScene)
                    {
                        std::string prefabKey = path.stem().string();
                        GameObject* pClonedObj = PrefabManager::GetInstance()->Instantiate(prefabKey, pActiveScene);
                        if (!pClonedObj)
                        {
                            PrefabManager::GetInstance()->LoadPrefab(prefabKey, path.string());
                            pClonedObj = PrefabManager::GetInstance()->Instantiate(prefabKey, pActiveScene);
                        }
                    }
                }

                if (ImGui::MenuItem("Rename"))
                {
                    m_RenameTargetEntry = path;
                    strncpy_s(m_RenameBuffer, filenameStr.c_str(), sizeof(m_RenameBuffer));
                    m_OpenRenamePopup = true;
                }
                ImGui::EndPopup();
            }

            if (ImGui::BeginDragDropSource())
            {
                std::string pathStr = path.string();
                if (isPrefabFile)
                {
                    ImGui::SetDragDropPayload("DND_PREFAB_FILE", pathStr.c_str(), pathStr.size() + 1);
                    ImGui::Text("Spawn Prefab: %s", filenameStr.c_str());
                }
                else if (isSceneFile)
                {
                    ImGui::SetDragDropPayload("DND_SCENE_FILE", pathStr.c_str(), pathStr.size() + 1);
                    ImGui::Text("Load Scene: %s", filenameStr.c_str());
                }
                ImGui::EndDragDropSource();
            }
        }
    }
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_REORDER_OBJ"))
        {
            GameObject* draggedObj = *(GameObject**)payload->Data;
            if (draggedObj)
            {
                std::string prefabPath = (m_CurrentDirectory / (draggedObj->GetName() + ".prefab")).string();
                JsonSerializer::SavePrefab(draggedObj, prefabPath);
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void ContentBrowserPanel::HandleWindowContextMenu()
{
    if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Monster ScriptableObject"))
            {
                auto newSO = DataManager::GetInstance()->CreateMonsterSO("NewMonster", m_CurrentDirectory.string());
                if (newSO)
                {
                    EditorSystem::GetInstance()->SetSelectedScriptableObject(newSO.get());
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("New Scene"))
            {
                SceneManager::GetInstance()->CreateDefaultTemplateScene("NewScene");
                std::string savePath = (m_CurrentDirectory / "NewScene.scene").string();
                SceneManager::GetInstance()->SaveActiveScene(savePath);
            }
            if (ImGui::MenuItem("New Folder"))
            {
                std::filesystem::create_directories(m_CurrentDirectory / "NewFolder");
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
}

void ContentBrowserPanel::HandleRenameModal()
{
    if (m_OpenRenamePopup)
    {
        ImGui::OpenPopup("Rename File");
        m_OpenRenamePopup = false;
    }
    if (ImGui::BeginPopupModal("Rename File", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter new name:");
        ImGui::InputText("##NewFileName", m_RenameBuffer, sizeof(m_RenameBuffer));
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            std::filesystem::path newPath = m_RenameTargetEntry.parent_path() / m_RenameBuffer;
            if (!newPath.empty() && newPath != m_RenameTargetEntry)
            {
                std::filesystem::rename(m_RenameTargetEntry, newPath);
                Scene* pActiveScene = SceneManager::GetInstance()->GetActiveScene();
                if (pActiveScene && pActiveScene->GetSceneName() == m_RenameTargetEntry.stem().string())
                {
                    pActiveScene->SetSceneName(newPath.stem().string());
                }
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
