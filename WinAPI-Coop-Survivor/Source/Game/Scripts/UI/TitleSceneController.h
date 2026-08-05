#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class UIButtonComponent;

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

	UIButtonComponent* host_Btn = nullptr;
	UIButtonComponent* join_Btn = nullptr;
	UIButtonComponent* exit_Btn = nullptr;

private:
	void OnClickHostBtn();
	void OnClickJoinBtn();
	void OnClickExitBtn();
};