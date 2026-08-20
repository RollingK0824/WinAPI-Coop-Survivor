#pragma once
#include "Engine/Manager/GUISystem.h"
#include <string>
#include <atomic>
#include <thread>
#include <windows.h>

class MainMenuBarPanel : public IGUIPanel
{
public:
    MainMenuBarPanel();
    virtual ~MainMenuBarPanel();
    void Initialize();
    void Release();
    virtual void OnDrawGUI() override;

private:
    void BuildAndRunRelease();
    void RunReleaseExe();
    void LoadBuildSettings();
    void SaveBuildSettings();

    enum class BuildState { Idle, Building, Success, Failed };
    std::atomic<BuildState> m_buildState{ BuildState::Idle };
    std::string m_buildStatusMessage;
    std::thread m_buildThread;

    char m_customMSBuildPath[MAX_PATH] = { 0 };
    bool m_showMSBuildModal = false;
};