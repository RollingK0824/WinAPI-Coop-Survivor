#include "Engine/Core/pch.h"
#include "MainMenuBarPanel.h"
#include "Engine/Core/EngineKernel.h"
#include "Engine/Manager/SceneManager.h"

void MainMenuBarPanel::Initialize()
{
    GUISystem::GetInstance()->RegisterPanel(this);
}

void MainMenuBarPanel::Release()
{
    GUISystem::GetInstance()->UnRegisterPanel(this);
}

void MainMenuBarPanel::OnDrawGUI()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save Scene to JSON", "Ctrl+S"))
            {
                SceneManager::GetInstance()->SaveActiveScene();
            }
            ImGui::EndMenu();
        }

        ImGui::SameLine(ImGui::GetWindowWidth() * 0.45f);

        EnginePlayState playState = EngineKernel::GetInstance()->GetPlayState();

        if (playState == EnginePlayState::Edit)
        {
            if (ImGui::Button(" Play "))
            {
                SceneManager::GetInstance()->SavePlaySnapshot();
                EngineKernel::GetInstance()->SetPlayState(EnginePlayState::Play);
            }
        }
        else
        {
            if (ImGui::Button(" Stop "))
            {
                EngineKernel::GetInstance()->SetPlayState(EnginePlayState::Edit);
                SceneManager::GetInstance()->RestorePlaySnapshot();
            }
        }

        ImGui::EndMainMenuBar();
    }
}