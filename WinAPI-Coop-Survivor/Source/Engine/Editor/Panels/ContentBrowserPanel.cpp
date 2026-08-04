#include "Engine/Core/pch.h"
#include "ContentBrowserPanel.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Manager/JsonSerializer.h"
#include "Engine/Manager/PrefabManager.h"
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
            bool isSceneFile = (path.extension() == ".scene" || path.extension() == ".json");
            bool isPrefabFile = (path.extension() == ".prefab");
            if (ImGui::Selectable(("[File] " + filenameStr).c_str())) {}
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (isSceneFile)
                {
                    SceneManager::GetInstance()->LoadSceneFromFile(path.string());
                }
            }
            if (ImGui::BeginPopupContextItem())
            {
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
