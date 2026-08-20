#include "Engine/Core/pch.h"
#include "TitleSceneController.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/UIButtonComponent.h"
#include "Engine/Framework/Components/UI/UIInputFieldComponent.h"
#include "Game/UI/ErrorPopupController.h"

static ComponentRegistrar<TitleSceneController> registrar(EngineKey::CustomComponent::TitleSceneController.data());

TitleSceneController::TitleSceneController(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeComponent("IP InputField", &ip_InputField);
	ExposeComponent("Host Button", &host_Btn);
	ExposeComponent("Join Button", &join_Btn);
	ExposeComponent("Exit Button", &exit_Btn);
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

	if (errorPopup_Ctrl == nullptr && pScene != nullptr)
	{
		for (auto* obj : pScene->GetGameObjects())
		{
			if (obj)
			{
				auto* ctrl = obj->GetComponent<ErrorPopupController>();
				if (ctrl)
				{
					errorPopup_Ctrl = ctrl;
					break;
				}
			}
		}
	}

	NetworkManager::GetInstance()->SetOnConnResultCallback([this](ConnResultCode code) {
		if (code == ConnResultCode::SUCCESS)
		{
			SceneManager::GetInstance()->LoadSceneFromFile("Resources/Scenes/InGameScene.scene");
		}
		else if (code == ConnResultCode::ROOM_FULL)
		{
			ShowErrorPopup(L"접속 실패: 서버/세션 정원이 초과되었습니다 (Room Full!)");
		}
		else if (code == ConnResultCode::GAME_ALREADY_STARTED)
		{
			ShowErrorPopup(L"접속 실패: 이미 게임이 시작된 방입니다.");
		}
		else if (code == ConnResultCode::REJECTED || code == ConnResultCode::INVALID_VERSION)
		{
			ShowErrorPopup(L"접속 실패: 호스트 연결에 실패했습니다 (서버 응답 없음 / 거절됨)");
		}
	});
}

void TitleSceneController::OnClickHostBtn()
{
	NetworkManager::GetInstance()->StartHost(9000);
	SceneManager::GetInstance()->LoadSceneFromFile("Resources/Scenes/InGameScene.scene");
}

void TitleSceneController::OnClickJoinBtn()
{
	std::string targetIP = "127.0.0.1";
	if (ip_InputField != nullptr)
	{
		std::string input = ip_InputField->GetText();
		if (!input.empty())
		{
			targetIP = input;
		}
	}

	if (errorPopup_Ctrl)
	{
		errorPopup_Ctrl->ShowConnecting(L"호스트에 접속 중입니다...");
	}

	NetworkManager::GetInstance()->ConnectToHost(targetIP, 9000);
}

void TitleSceneController::OnClickExitBtn()
{
	PostQuitMessage(0);
}

void TitleSceneController::ShowErrorPopup(const std::wstring& message)
{
	if (errorPopup_Ctrl)
	{
		errorPopup_Ctrl->ShowError(message);
	}
	else
	{
		MessageBoxW(nullptr, message.c_str(), L"Connection Error", MB_OK | MB_ICONERROR);
	}
}
