#include "Engine/Core/pch.h"
#include "ContentBrowserPanel.h"

void ContentBrowserPanel::Initialize()
{
    GUISystem::GetInstance()->RegisterPanel(this);
}

void ContentBrowserPanel::Release()
{
    GUISystem::GetInstance()->UnRegisterPanel(this);
}

void ContentBrowserPanel::OnDrawGUI()
{
    ImGui::Begin("Content Browser");

    // 상위 폴더로 이동 버튼
    if (m_CurrentDirectory != std::filesystem::path("Resources"))
    {
        if (ImGui::Button("<- Back"))
        {
            m_CurrentDirectory = m_CurrentDirectory.parent_path();
        }
    }

    // 디렉토리 및 파일 탐색 목록 출력
    if (std::filesystem::exists(m_CurrentDirectory))
    {
        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = directoryEntry.path();
            std::string filenameString = path.filename().string();

            if (directoryEntry.is_directory())
            {
                if (ImGui::Button(("[Folder] " + filenameString).c_str()))
                {
                    m_CurrentDirectory /= path.filename();
                }
            }
            else
            {
                ImGui::Text(("[File] " + filenameString).c_str());
            }
        }
    }

    ImGui::End();
}