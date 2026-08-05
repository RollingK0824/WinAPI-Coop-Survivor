#pragma once
#include "Engine/Manager/GUISystem.h"

class ContentBrowserPanel : public IGUIPanel
{
public:
    ContentBrowserPanel() = default;
    virtual ~ContentBrowserPanel() = default;

    void Initialize();
    void Release();

    virtual void OnDrawGUI() override;

private:
    void DrawTopBar();
    void DrawContentGrid();
    void HandleWindowContextMenu();
    void HandleRenameModal();
private:
    std::filesystem::path m_CurrentDirectory = "Resources";

    std::filesystem::path m_RenameTargetEntry;
    char m_RenameBuffer[256] = "";
    bool m_OpenRenamePopup = false;
};