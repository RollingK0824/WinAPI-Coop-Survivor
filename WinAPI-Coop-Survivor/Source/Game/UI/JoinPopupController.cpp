#include "Engine/Core/pch.h"
#include "JoinPopupController.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/UIButtonComponent.h"
#include "Engine/Framework/Components/UI/UIInputFieldComponent.h"

static ComponentRegistrar<JoinPopupController> registrar("JoinPopupController");

JoinPopupController::JoinPopupController(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeComponent("Connect Button", &connect_Btn);
	ExposeComponent("Cancel Button", &cancel_Btn);
	ExposeComponent("IP Input Field", &ip_InputField);
	ExposeComponent("Port Input Field", &port_InputField);
}

void JoinPopupController::Start()
{
	ScriptComponent::Start();

	if (connect_Btn)
	{
		connect_Btn->SetOnClick([this]() { OnClickConnect(); });
	}

	if (cancel_Btn)
	{
		cancel_Btn->SetOnClick([this]() { OnClickCancel(); });
	}
}

void JoinPopupController::ShowPopup(bool show)
{
	gameObject.SetActive(show);
	if (connect_Btn) connect_Btn->gameObject.SetActive(show);
	if (cancel_Btn) cancel_Btn->gameObject.SetActive(show);
	if (ip_InputField) ip_InputField->gameObject.SetActive(show);
	if (port_InputField) port_InputField->gameObject.SetActive(show);
}

void JoinPopupController::OnClickConnect()
{
	std::string ip = ip_InputField ? ip_InputField->GetText() : "127.0.0.1";
	int port = port_InputField ? port_InputField->GetIntValue(9000) : 9000;

	if (ip.empty()) ip = "127.0.0.1";

	if (NetworkManager::GetInstance()->ConnectToHost(ip, port))
	{
		ShowPopup(false);
		SceneManager::GetInstance()->LoadSceneFromFile("Resources/Scenes/InGameScene.scene");
	}
}

void JoinPopupController::OnClickCancel()
{
	ShowPopup(false);
}
