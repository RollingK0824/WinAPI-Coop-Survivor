#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class UIButtonComponent;

class GameOverSceneController : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(GameOverSceneController)

	GameOverSceneController(GameObject* owner, TransformComponent* transform);
	virtual ~GameOverSceneController() override = default;

	virtual void Start() override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::GameOverSceneController;
	}

	void OnClickOkBtn();

private:
	UIButtonComponent* m_okBtn = nullptr;
};
