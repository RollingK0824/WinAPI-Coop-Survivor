#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class UIButtonComponent;
class UIInputFieldComponent;

class HostPopupController : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(HostPopupController)

	HostPopupController(GameObject* owner, TransformComponent* transform);
	virtual ~HostPopupController() override = default;

	virtual void Start() override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::HostPopupController;
	}

	void ShowPopup(bool show);

	UIButtonComponent* startHost_Btn = nullptr;
	UIButtonComponent* cancel_Btn = nullptr;
	UIInputFieldComponent* port_InputField = nullptr;

private:
	void OnClickStartHost();
	void OnClickCancel();
};
