#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/ObserverPtr.h"

class UIButtonComponent;
class UIInputFieldComponent;
class ErrorPopupController;

class TitleSceneController : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(TitleSceneController)

	TitleSceneController(GameObject* owner, TransformComponent* transform);
	virtual ~TitleSceneController() override = default;

	virtual void Start() override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::TitleSceneController;
	}

	UIInputFieldComponent* ip_InputField = nullptr;
	UIButtonComponent* host_Btn = nullptr;
	UIButtonComponent* join_Btn = nullptr;
	UIButtonComponent* exit_Btn = nullptr;

	ErrorPopupController* errorPopup_Ctrl = nullptr;

private:
	void OnClickHostBtn();
	void OnClickJoinBtn();
	void OnClickExitBtn();

	void ShowErrorPopup(const std::wstring& message);
};
