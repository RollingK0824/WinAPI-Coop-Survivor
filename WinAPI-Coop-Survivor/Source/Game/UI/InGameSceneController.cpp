#include "Engine/Core/pch.h"
#include "InGameSceneController.h"
#include "Engine/Core/ComponentRegister.h"
#include "Game/Manager/InGameManager.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Framework/Components/UI/UIButtonComponent.h"
#include "Engine/Framework/Components/UI/UITextComponent.h"

static ComponentRegistrar<InGameSceneController> registrar(EngineKey::CustomComponent::InGameSceneController.data());

InGameSceneController::InGameSceneController(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner,transform)
{
	ExposeComponent("Action Button", &m_actionBtn);
	ExposeComponent("Action Button Text", &m_actionBtnText);
	ExposeComponent("Status Text", &m_statusText);
	ExposeComponent("Countdown Text", &m_countdownText);
}

void InGameSceneController::Start()
{
	ScriptComponent::Start();

	InGameManager* inGameMgr = InGameManager::GetInstance();
	NetRole role = NetworkManager::GetInstance()->GetRole();

	if (m_actionBtn) m_actionBtn->SetOnClick([this]() { OnClickActionBtn(); });

	if (m_actionBtnText)
	{
		if (role == NetRole::HOST)
		{
			m_actionBtnText->SetText(L"START GAME");
		}
		else if (role == NetRole::CLIENT)
		{
			m_actionBtnText->SetText(L"READY");
		}
	}
	
	if (m_countdownText)
	{
		m_countdownText->gameObject.SetActive(false);
	}

	if (!inGameMgr) return;

	inGameMgr->SetOnCountdownTickCallback([this](float remainingTime) {
		if (m_actionBtn) m_actionBtn->gameObject.SetActive(false);
		if (m_statusText) m_statusText->gameObject.SetActive(false);

		if (m_countdownText)
		{
			m_countdownText->gameObject.SetActive(true);
			if (remainingTime > 0.1f)
			{
				std::wstring str = L"GAME STARTS IN: " + std::to_wstring(static_cast<int>(std::ceil(remainingTime)));
				m_countdownText->SetText(str);
			}
			else
			{
				m_countdownText->SetText(L"GO!!!");
			}
		}
	});

	inGameMgr->SetOnGameStartedCallback([this]() {
		if (m_countdownText) m_countdownText->gameObject.SetActive(false);
	});

	inGameMgr->SetOnReadyStatusChangedCallback([this, role](bool allReady) {
		if (role == NetRole::HOST && m_actionBtn)
		{
			m_actionBtn->SetEnable(allReady);
			if (m_statusText)
			{
				m_statusText->SetText(allReady ? L"All Players Ready" : L"Waiting for Clients...");
			}
		}
	});
}

void InGameSceneController::OnClickActionBtn()
{
	InGameManager* inGameMgr = InGameManager::GetInstance();
	if (!inGameMgr) return;

	NetRole role = NetworkManager::GetInstance()->GetRole();

	if (role == NetRole::HOST)
	{
		if (inGameMgr->IsAllClientsReady())
		{
			inGameMgr->SendHostStartSignal();
		}
	}
	else
	{
		m_bIsClientReady = !m_bIsClientReady;
		inGameMgr->SendClientReadyStatus(m_bIsClientReady);

		if (m_actionBtnText)
		{
			m_actionBtnText->SetText(m_bIsClientReady ? L"CANCEL READY" : L"READY");
		}
	}
}