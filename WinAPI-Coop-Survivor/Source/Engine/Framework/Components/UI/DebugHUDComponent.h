#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class HUDPresenter;

class DebugHUDComponent : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(DebugHUDComponent)
	DebugHUDComponent(GameObject* owner, TransformComponent* transform);
	virtual ~DebugHUDComponent() override = default;

	virtual void Awake() override;

	virtual void OnDestroy() override;

	void UpdateDebugData(const std::string& title, 
		const std::vector<std::pair<std::string, std::string>>& data);
private:
	HUDPresenter* m_pPresenter = nullptr;
};