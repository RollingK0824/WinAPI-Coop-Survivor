#include "Engine/Core/pch.h"
#include "HierarchyPanel.h"
#include "Engine/Editor/EditorSystem.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Manager/JsonSerializer.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"

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
        DrawSceneHeader(pActiveScene);
        DrawGameObjectList(pActiveScene);
        HandlePrefabDrop(pActiveScene);
        HandleWindowContextMenu(pActiveScene);
    }
    ImGui::End();
}

void HierarchyPanel::DrawSceneHeader(Scene* pActiveScene)
{
    std::string sceneName = pActiveScene->GetSceneName();
    if (sceneName.empty()) sceneName = "Untitled Scene";
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 1.0f, 1.0f));
    ImGui::Text("[Scene] %s", sceneName.c_str());
    ImGui::PopStyleColor();
    ImGui::Separator();
}

void HierarchyPanel::DrawGameObjectList(Scene* pActiveScene)
{
    const auto& objects = pActiveScene->GetGameObjects();
    for (GameObject* pObj : objects)
    {
        if (!pObj || pObj->IsDead()) continue;
        
        if (pObj->GetParent() != nullptr) continue;
        DrawGameObjectNode(pActiveScene, pObj);
    }
}

void HierarchyPanel::DrawGameObjectNode(Scene* pActiveScene, GameObject* pObj)
{
    if (!pObj || pObj->IsDead()) return;

    ImGui::PushID(pObj);

    bool isSelected = (EditorSystem::GetInstance()->GetSelectedObject() == pObj);
    bool hasChildren = !pObj->GetChildren().empty();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                             | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (isSelected)    flags |= ImGuiTreeNodeFlags_Selected;
    if (!hasChildren)  flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    bool nodeOpen = ImGui::TreeNodeEx(pObj->GetName().c_str(), flags);

    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen() && ImGui::GetDragDropPayload() == nullptr)
    {
        EditorSystem::GetInstance()->SetSelectedObject(pObj);
    }

    HandleItemContextMenu(pActiveScene, pObj);

    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("HIERARCHY_GO", &pObj, sizeof(GameObject*));
        ImGui::Text("Move: %s", pObj->GetName().c_str());
        ImGui::EndDragDropSource();
    }
    
    HandleDragAndDropParenting(pActiveScene, pObj);

    if (nodeOpen && hasChildren)
    {
        for (GameObject* pChild : pObj->GetChildren())
        {
            DrawGameObjectNode(pActiveScene, pChild);
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void HierarchyPanel::HandleItemContextMenu(Scene* pActiveScene, GameObject* pObj)
{
    if (ImGui::BeginPopupContextItem())
    {
        EditorSystem::GetInstance()->SetSelectedObject(pObj);

        if (ImGui::MenuItem("Delete GameObject"))
        {
            pActiveScene->DestroyObjects(pObj);
            if (EditorSystem::GetInstance()->GetSelectedObject() == pObj)
            {
                EditorSystem::GetInstance()->SetSelectedObject(nullptr);
            }
        }

        if (ImGui::MenuItem("Save as Prefab"))
        {
            std::string prefabPath = "Resources/Prefabs/" + pObj->GetName() + ".prefab";
            JsonSerializer::SavePrefab(pObj, prefabPath);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Create Empty Child"))
        {
            GameObject* pChild = pActiveScene->CreateGameObject("New GameObject");
            if (pChild)
                pChild->SetParent(pObj, false);
        }

        if (pObj->GetParent() != nullptr)
        {
            if (ImGui::MenuItem("Unparent"))
            {
                pObj->SetParent(nullptr, true);
            }
        }

        ImGui::EndPopup();
    }
}

void HierarchyPanel::HandleDragAndDropParenting(Scene* pActiveScene, GameObject* pTargetObj)
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_GO"))
        {
            GameObject* pDragged = *(GameObject**)payload->Data;
            if (pDragged && pDragged != pTargetObj && !pTargetObj->IsDescendantOf(pDragged))
            {
                pDragged->SetParent(pTargetObj, true);
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void HierarchyPanel::HandlePrefabDrop(Scene* pActiveScene)
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_PREFAB_FILE"))
        {
            const char* prefabFilePath = (const char*)payload->Data;
            
            std::ifstream file(prefabFilePath);
            if (file.is_open())
            {
                json prefabJson;
                file >> prefabJson;
                JsonSerializer::InstantiateFromPrefabData(pActiveScene, prefabJson);
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void HierarchyPanel::HandleWindowContextMenu(Scene* pActiveScene)
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_GO"))
        {
            GameObject* pDragged = *(GameObject**)payload->Data;
            if (pDragged)
                pDragged->SetParent(nullptr, true);
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Create Empty GameObject"))
        {
            pActiveScene->CreateGameObject("New GameObject");
        }
        ImGui::EndPopup();
    }
}
