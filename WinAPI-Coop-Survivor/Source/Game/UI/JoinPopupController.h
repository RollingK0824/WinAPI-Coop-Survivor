#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class UIButtonComponent;
class UIInputFieldComponent;

class JoinPopupController : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(JoinPopupController)

	JoinPopupController(GameObject* owner, TransformComponent* transform);
	virtual ~JoinPopupController() override = default;

	virtual void Start() override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::JoinPopupController;
	}

	void ShowPopup(bool show);

	UIButtonComponent* connect_Btn = nullptr;
	UIButtonComponent* cancel_Btn = nullptr;
	UIInputFieldComponent* ip_InputField = nullptr;
	UIInputFieldComponent* port_InputField = nullptr;

private:
	void OnClickConnect();
	void OnClickCancel();
};
