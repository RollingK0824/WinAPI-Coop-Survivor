#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class UITextComponent;

class HUDPresenter : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(HUDPresenter)

	HUDPresenter(GameObject* owner, TransformComponent* transform);
	virtual ~HUDPresenter() override = default;

	virtual void Awake() override;

	void SetData(const std::string& title, 
		const std::vector < std::pair<std::string, std::string>>& data);

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::HUDPresenter;
	}

private:
	UITextComponent* m_pTextView = nullptr;
};