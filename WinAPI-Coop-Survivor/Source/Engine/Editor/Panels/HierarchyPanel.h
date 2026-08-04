#pragma once
#include "Engine/Manager/GUISystem.h"

class HierarchyPanel : public IGUIPanel
{
public:
    HierarchyPanel() = default;
    virtual ~HierarchyPanel() = default;
    void Initialize();
    void Release();
    virtual void OnDrawGUI() override;
};