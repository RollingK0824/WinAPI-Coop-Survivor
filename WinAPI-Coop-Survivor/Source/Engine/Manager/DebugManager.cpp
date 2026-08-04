#include "Engine/Core/pch.h"
#include "DebugManager.h"
#include "Engine/Manager/ActionManager.h"
#include "Engine/Manager/TimeManager.h"
#include "Engine/Manager/PrefabManager.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Renderer/RenderSystem.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/UIPanelComponent.h"
#include "Engine/Framework/Components/UI/UITextComponent.h"
#include "Engine/Framework/Components/UI/UIImageComponent.h"
#include "Engine/Framework/Components/UI/HUDPresenter.h"
#include "Engine/Framework/Components/UI/DebugHUDComponent.h"

bool DebugManager::Initialize()
{
    ActionManager::GetInstance()->BindAction(m_actionName, VK_F3);
    return true;
}

void DebugManager::Update(float dt)
{
    if (ActionManager::GetInstance()->GetActionDown(m_actionName))
    {
        SetVisible(!m_bVisible);
    }

    if (!m_bVisible || !m_pDebugHUDComp) return;

    uint32 fps = TimeManager::GetInstance()->GetFPS();
    float gameTime = TimeManager::GetInstance()->GetGameTime();
    int32 min = static_cast<int32>(gameTime) / 60;
    int32 sec = static_cast<int32>(gameTime) % 60;

    NetRole role = NetworkManager::GetInstance()->GetRole();
    uint32 myNetID = NetworkManager::GetInstance()->GetMyNetID();
    float ping = NetworkManager::GetInstance()->GetPing();
    bool bIsConnected = NetworkManager::GetInstance()->IsConnected();

    std::string roleStr = (role == NetRole::HOST) ? "HOST" : (role == NetRole::CLIENT) ? "CLIENT" : "OFFLINE";
    std::string pingStr = bIsConnected ? std::format("{:.1f}ms", ping) : "Connecting...";

    std::vector < std::pair < std::string, std::string>> debugData = {
        {"FPS",std::to_string(fps)},
        {"Time",std::format("{:02d}:{:02d}",min,sec)},
        {"Role",roleStr},
        {"NetID",std::to_string(myNetID)},
        {"Ping",pingStr}
    };

    m_pDebugHUDComp->UpdateDebugData("Debug HUD", debugData);
}

void DebugManager::SetVisible(bool visible)
{
    m_bVisible = visible;
    if (m_pDebugUIRoot)
    {
        m_pDebugUIRoot->SetActive(m_bVisible);
    }
}

GameObject* DebugManager::CreateDebugUIOverlay(Scene* pScene)
{
    if (!pScene)return nullptr;

    GameObject* pDebugObj = pScene->CreateGameObject("DebugHUD_Root");

    auto* pPanel = pDebugObj->AddComponent<UIPanelComponent>();
    if (pPanel)
    {
        pPanel->SetSize({ 220.0f, 150.0f });
        pPanel->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.6f)); // 반투명 검은색
    }

    auto* pText = pDebugObj->AddComponent<UITextComponent>();
    if (pText)
    {
        pText->SetFontSize(14.0f);
        pText->SetColor(D2D1::ColorF(D2D1::ColorF::Green)); // 연두색 텍스트
    }

    pDebugObj->AddComponent<HUDPresenter>();
    auto* pDebugHUD = pDebugObj->AddComponent<DebugHUDComponent>();

    return pDebugObj;
}

void DebugManager::RegisterDebugHUD(DebugHUDComponent* pComp, GameObject* pRootObj)
{
    m_pDebugHUDComp = pComp;
    m_pDebugUIRoot = pRootObj;
    if (m_pDebugUIRoot)
    {
        m_pDebugUIRoot->SetActive(m_bVisible);
    }
}

void DebugManager::UnRegisterDebugHUD(DebugHUDComponent* pComp)
{
    if (m_pDebugHUDComp == pComp)
    {
        m_pDebugHUDComp = nullptr;
        m_pDebugUIRoot = nullptr;
    }
}

