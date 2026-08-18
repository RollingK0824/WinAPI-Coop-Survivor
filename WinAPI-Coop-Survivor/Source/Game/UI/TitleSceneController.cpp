#include "Engine/Core/pch.h"
#include "TitleSceneController.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/UIButtonComponent.h"
#include "Game/UI/HostPopupController.h"
#include "Game/UI/JoinPopupController.h"
#include "Game/UI/ErrorPopupController.h"

static ComponentRegistrar<TitleSceneController> registrar(EngineKey::CustomComponent::TitleSceneController.data());

TitleSceneController::TitleSceneController(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeComponent("Host Button", &host_Btn);
	ExposeComponent("Join Button", &join_Btn);
	ExposeComponent("Exit Button", &exit_Btn);
	ExposeComponent("Host Popup Controller", &hostPopup_Ctrl);
	ExposeComponent("Join Popup Controller", &joinPopup_Ctrl);
	ExposeComponent("Error Popup Controller", &errorPopup_Ctrl);
}

void TitleSceneController::Start()
{
	ScriptComponent::Start();

	Scene* pScene = gameObject.GetOwnerScene();

	if (host_Btn != nullptr)
	{
		host_Btn->SetOnClick([this]() { OnClickHostBtn(); });
	}

	if (join_Btn != nullptr)
	{
		join_Btn->SetOnClick([this]() { OnClickJoinBtn(); });
	}

	if (exit_Btn != nullptr)
	{
		exit_Btn->SetOnClick([this]() { OnClickExitBtn(); });
	}

	// 씬 내 사전 배치된 컨트롤러 바인딩 백업
	if (pScene)
	{
		if (!hostPopup_Ctrl)
		{
			if (GameObject* hostObj = pScene->FindGameObjectByName("HostPopup_BG"))
			{
				hostPopup_Ctrl = hostObj->GetComponent<HostPopupController>();
			}
		}

		if (!joinPopup_Ctrl)
		{
			if (GameObject* joinObj = pScene->FindGameObjectByName("JoinPopup_BG"))
			{
				joinPopup_Ctrl = joinObj->GetComponent<JoinPopupController>();
			}
		}

		if (!errorPopup_Ctrl)
		{
			if (GameObject* errObj = pScene->FindGameObjectByName("ErrorPopup_BG"))
			{
				errorPopup_Ctrl = errObj->GetComponent<ErrorPopupController>();
			}
		}
	}

	// 네트워크 연결 결과 콜백 등록 (Room Full 또는 접속 실패 시 에러 팝업 표시)
	NetworkManager::GetInstance()->SetOnConnResultCallback([this](ConnResultCode code) {
		if (code == ConnResultCode::ROOM_FULL)
		{
			ShowErrorPopup(L"접속 실패: 서버/세션 정원이 초과되었습니다 (Room Full!)");
		}
		else if (code == ConnResultCode::REJECTED || code == ConnResultCode::INVALID_VERSION)
		{
			ShowErrorPopup(L"접속 실패: 호스트 연결이 거절되었습니다.");
		}
	});
}

void TitleSceneController::OnClickHostBtn()
{
	ShowHostPopup();
}

void TitleSceneController::OnClickJoinBtn()
{
	ShowJoinPopup();
}

void TitleSceneController::OnClickExitBtn()
{
	PostQuitMessage(0);
}

void TitleSceneController::ShowHostPopup()
{
	if (hostPopup_Ctrl)
	{
		hostPopup_Ctrl->ShowPopup(true);
	}
}

void TitleSceneController::ShowJoinPopup()
{
	if (joinPopup_Ctrl)
	{
		joinPopup_Ctrl->ShowPopup(true);
	}
}

void TitleSceneController::ShowErrorPopup(const std::wstring& message)
{
	if (errorPopup_Ctrl)
	{
		errorPopup_Ctrl->ShowError(message);
	}
}
