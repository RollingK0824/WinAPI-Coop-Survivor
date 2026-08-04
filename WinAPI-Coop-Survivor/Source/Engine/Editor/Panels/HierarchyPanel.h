#pragma once
#include "Engine/Manager/GUISystem.h"

class Scene;
class GameObject;

class HierarchyPanel : public IGUIPanel
{
public:
    HierarchyPanel() = default;
    virtual ~HierarchyPanel() = default;
    void Initialize();
    void Release();
    virtual void OnDrawGUI() override;

private:
    void DrawSceneHeader(Scene* pActiveScene);
    void DrawGameObjectList(Scene* pActiveScene);
    void DrawGameObjectNode(Scene* pActiveScene, GameObject* pObj, int index);

    void HandleItemContextMenu(Scene* pActiveScene, GameObject* pObj);
    void HandleDragAndDropReorder(GameObject* pTargetObj, int targetIndex);
    void HandlePrefabDrop(Scene* pActiveScene);
    void HandleWindowContextMenu(Scene* pActiveScene);
};