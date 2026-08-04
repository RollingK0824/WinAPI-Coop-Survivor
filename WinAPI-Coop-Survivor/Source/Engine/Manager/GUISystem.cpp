#include "Engine/Core/pch.h"
#include "GUISystem.h"
#include "Engine/Core/GameApp.h"
#include "Engine/Renderer/GraphicManager.h"

bool GUISystem::Initialize()
{
    if (m_bInitialized) return true;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    HWND hWnd = GameApp::GetInstance()->GetWindowHandle();
    GraphicManager* pGM = GraphicManager::GetInstance();

    if (!hWnd || !pGM->GetD3DDevice() || !pGM->GetD3DContext())
    {
        return false;
    }

    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(pGM->GetD3DDevice(), pGM->GetD3DContext());

    m_bInitialized = true;
    return true;
}

void GUISystem::Release()
{
    if (!m_bInitialized)return;

    m_vPanels.clear();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    m_bInitialized = false;
}

void GUISystem::Update(float dt)
{
    if (!m_bInitialized)return;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
}

void GUISystem::Render()
{
    if (!m_bInitialized)return;

    GraphicManager::GetInstance()->EndD2DDraw();

    for (auto* pPanel : m_vPanels)
    {
        if (pPanel) pPanel->OnDrawGUI();
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ID3D11DeviceContext* pCtx = GraphicManager::GetInstance()->GetD3DContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void GUISystem::RegisterPanel(IGUIPanel* pPanel)
{
    if (pPanel == nullptr)return;
    auto it = std::find(m_vPanels.begin(), m_vPanels.end(), pPanel);
    if (it == m_vPanels.end())
    {
        m_vPanels.push_back(pPanel);
    }
}

void GUISystem::UnRegisterPanel(IGUIPanel* pPanel)
{
    if (pPanel == nullptr)return;

    auto it = std::find(m_vPanels.begin(), m_vPanels.end(), pPanel);
    if (it != m_vPanels.end())
    {
        m_vPanels.erase(it);
    }
}
