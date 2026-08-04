#pragma once
#include "Engine/Manager/GUISystem.h"

class MainMenuBarPanel : public IGUIPanel
{
public:
    MainMenuBarPanel() = default;
    virtual ~MainMenuBarPanel() = default;
    void Initialize();
    void Release();
    virtual void OnDrawGUI() override;
};