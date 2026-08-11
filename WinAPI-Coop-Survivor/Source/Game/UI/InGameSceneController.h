#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class UIButtonComponent;
class UIImageComponent;
class UITextComponent;

class InGameSceneController : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(InGameSceneController)

	InGameSceneController(GameObject* owner, TransformComponent* transform);
	virtual ~InGameSceneController() override = default;

	virtual void Start() override;
	void OnClickActionBtn();

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::InGameSceneController;
	}

private:
	UIButtonComponent* m_actionBtn = nullptr;
	UITextComponent* m_actionBtnText = nullptr;
	UITextComponent* m_statusText = nullptr;
	UITextComponent* m_countdownText = nullptr;

	bool m_bIsClientReady = false;
};