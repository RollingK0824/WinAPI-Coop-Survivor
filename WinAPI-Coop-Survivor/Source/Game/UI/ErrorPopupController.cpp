#include "Engine/Core/pch.h"
#include "ErrorPopupController.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/UIButtonComponent.h"
#include "Engine/Framework/Components/UI/UITextComponent.h"

static ComponentRegistrar<ErrorPopupController> registrar("ErrorPopupController");

ErrorPopupController::ErrorPopupController(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeComponent("OK Button", &ok_Btn);
	ExposeComponent("Message Text", &messageText_Comp);
}

void ErrorPopupController::Start()
{
	ScriptComponent::Start();

	if (ok_Btn)
	{
		ok_Btn->SetOnClick([this]() { OnClickOK(); });
	}
}

void ErrorPopupController::ShowMessage(const std::wstring& message, bool showOkButton)
{
	if (messageText_Comp)
	{
		messageText_Comp->SetText(message);
	}

	gameObject.SetActive(true);

	if (ok_Btn)
	{
		ok_Btn->gameObject.SetActive(showOkButton);
	}
}

void ErrorPopupController::ShowError(const std::wstring& message)
{
	ShowMessage(message, true);
}

void ErrorPopupController::ShowConnecting(const std::wstring& message)
{
	ShowMessage(message, false);
}

void ErrorPopupController::HidePopup()
{
	gameObject.SetActive(false);
}

void ErrorPopupController::OnClickOK()
{
	HidePopup();
}
