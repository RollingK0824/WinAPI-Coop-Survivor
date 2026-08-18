#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include <string>

class UIButtonComponent;
class UITextComponent;

class ErrorPopupController : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(ErrorPopupController)

	ErrorPopupController(GameObject* owner, TransformComponent* transform);
	virtual ~ErrorPopupController() override = default;

	virtual void Start() override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::ErrorPopupController;
	}

	void ShowError(const std::wstring& message);
	void HidePopup();

	UIButtonComponent* ok_Btn = nullptr;
	UITextComponent* messageText_Comp = nullptr;

private:
	void OnClickOK();
};
