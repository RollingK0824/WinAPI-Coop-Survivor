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
    void DrawGameObjectNode(Scene* pActiveScene, GameObject* pObj);

    void HandleItemContextMenu(Scene* pActiveScene, GameObject* pObj);
    void HandleDragAndDropParenting(Scene* pActiveScene, GameObject* pTargetObj);
    void HandlePrefabDrop(Scene* pActiveScene);
    void HandleWindowContextMenu(Scene* pActiveScene);
};