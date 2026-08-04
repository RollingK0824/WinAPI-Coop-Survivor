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
    std::filesystem::path m_CurrentDirectory = "Resources";
};