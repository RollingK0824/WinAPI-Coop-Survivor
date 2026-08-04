#include "Engine/Core/pch.h"
#include "EditorSystem.h"
#include "Engine/Core/GameApp.h"
#include "Engine/Core/EngineKernel.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Manager/JsonSerializer.h"
#include "Engine/Manager/InputManager.h"
#include "Engine/Manager/ActionManager.h"
#include "Engine/Manager/CameraManager.h"
#include "Engine/Editor/Panels/MainMenuBarPanel.h"
#include "Engine/Editor/Panels/HierarchyPanel.h"
#include "Engine/Editor/Panels/InspectorPanel.h"
#include "Engine/Editor/Panels/ContentBrowserPanel.h"
#include "Engine/Editor/Panels/ViewportPanel.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/Components/Core/CameraComponent.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"

EditorSystem::EditorSystem() = default;
EditorSystem::~EditorSystem() = default;

bool EditorSystem::Initialize()
{

#if WITH_EDITOR
    ActionManager::GetInstance()->BindShortcut("SaveScene", 'S', { VK_CONTROL });
#endif

    m_pMainMenuBarPanel = std::make_unique<MainMenuBarPanel>();
    if (m_pMainMenuBarPanel)m_pMainMenuBarPanel->Initialize();

    m_pHierarchyPanel = std::make_unique<HierarchyPanel>();
    if (m_pHierarchyPanel)m_pHierarchyPanel->Initialize();

    m_pInspectorPanel = std::make_unique<InspectorPanel>();
    if (m_pInspectorPanel)m_pInspectorPanel->Initialize();

    m_pContentBrowserPanel = std::make_unique<ContentBrowserPanel>();
    if (m_pContentBrowserPanel)m_pContentBrowserPanel->Initialize();

    m_pViewportPanel = std::make_unique<ViewportPanel>();
    if (m_pViewportPanel) m_pViewportPanel->Initialize();
     
    return true;
}

void EditorSystem::Release()
{
    if (m_pMainMenuBarPanel) m_pMainMenuBarPanel->Release();
    if (m_pHierarchyPanel) m_pHierarchyPanel->Release();
    if (m_pInspectorPanel) m_pInspectorPanel->Release();
    if (m_pContentBrowserPanel) m_pContentBrowserPanel->Release();
    if (m_pViewportPanel) m_pViewportPanel->Release();

    m_pMainMenuBarPanel.reset();
    m_pHierarchyPanel.reset();
    m_pInspectorPanel.reset();
    m_pContentBrowserPanel.reset();
    m_pViewportPanel.reset();

    m_pSelectedObject = nullptr;
}

void EditorSystem::Update(float dt)
{
    if (ActionManager::GetInstance()->GetActionDown("SaveScene"))
    {
        SceneManager::GetInstance()->SaveActiveScene();
    }

    if (EngineKernel::GetInstance()->GetPlayState() != EnginePlayState::Edit) return;
    CameraComponent* pMainCamera = CameraManager::GetInstance()->GetMainCamera();
    if (!pMainCamera) return;
    static Vector2 lastMousePos = { 0.0f, 0.0f };
    POINT curPoint;
    GetCursorPos(&curPoint);
    ScreenToClient(GameApp::GetInstance()->GetWindowHandle(), &curPoint);
    Vector2 currentMousePos((float)curPoint.x, (float)curPoint.y);
    if (InputManager::GetInstance()->GetKeyPress(VK_RBUTTON) || InputManager::GetInstance()->GetKeyPress(VK_MBUTTON))
    {
        Vector2 mouseDelta = currentMousePos - lastMousePos;
        float zoom = pMainCamera->GetZoom();
        Vector2 camPos = pMainCamera->transform.GetPosition();

        camPos.x -= (mouseDelta.x / zoom);
        camPos.y -= (mouseDelta.y / zoom);
        pMainCamera->transform.SetPosition(camPos);
    }
    lastMousePos = currentMousePos;
}
