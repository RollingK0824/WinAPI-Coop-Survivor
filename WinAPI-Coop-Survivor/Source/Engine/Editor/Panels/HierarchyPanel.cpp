#include "Engine/Core/pch.h"
#include "HierarchyPanel.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Editor/EditorSystem.h"

void HierarchyPanel::Initialize()
{
    GUISystem::GetInstance()->RegisterPanel(this);
}

void HierarchyPanel::Release()
{
    GUISystem::GetInstance()->UnRegisterPanel(this);
}

void HierarchyPanel::OnDrawGUI()
{
    ImGui::Begin("Hierarchy");
    Scene* pActiveScene = SceneManager::GetInstance()->GetActiveScene();
    if (pActiveScene)
    {
        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem("Create Empty GameObject"))
            {
                pActiveScene->CreateGameObject("New GameObject");
            }
            ImGui::EndPopup();
        }
        for (GameObject* pObj : pActiveScene->GetGameObjects())
        {
            if (!pObj) continue;
            bool isSelected = (EditorSystem::GetInstance()->GetSelectedObject() == pObj);
            if (ImGui::Selectable(pObj->GetName().c_str(), isSelected))
            {
                EditorSystem::GetInstance()->SetSelectedObject(pObj);
            }
        }
    }
    ImGui::End();
}