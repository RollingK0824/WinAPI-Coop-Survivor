#include "Engine/Core/pch.h"
#include "HostPopupController.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/UIButtonComponent.h"
#include "Engine/Framework/Components/UI/UIInputFieldComponent.h"

static ComponentRegistrar<HostPopupController> registrar("HostPopupController");

HostPopupController::HostPopupController(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeComponent("Start Host Button", &startHost_Btn);
	ExposeComponent("Cancel Button", &cancel_Btn);
	ExposeComponent("Port Input Field", &port_InputField);
}

void HostPopupController::Start()
{
	ScriptComponent::Start();

	if (startHost_Btn)
	{
		startHost_Btn->SetOnClick([this]() { OnClickStartHost(); });
	}

	if (cancel_Btn)
	{
		cancel_Btn->SetOnClick([this]() { OnClickCancel(); });
	}
}

void HostPopupController::ShowPopup(bool show)
{
	gameObject.SetActive(show);
}

void HostPopupController::OnClickStartHost()
{
	int port = port_InputField ? port_InputField->GetIntValue(9000) : 9000;
	if (NetworkManager::GetInstance()->StartHost(port))
	{
		ShowPopup(false);
		SceneManager::GetInstance()->LoadSceneFromFile("Resources/Scenes/InGameScene.scene");
	}
}

void HostPopupController::OnClickCancel()
{
	ShowPopup(false);
}
