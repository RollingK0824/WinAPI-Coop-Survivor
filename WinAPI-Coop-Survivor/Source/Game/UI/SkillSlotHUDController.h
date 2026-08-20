#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/Define.h"

class UIImageComponent;
class Player;

class SkillSlotHUDController : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(SkillSlotHUDController)

	SkillSlotHUDController(GameObject* owner, TransformComponent* transform);
	virtual ~SkillSlotHUDController() override = default;

	virtual void Start() override;
	virtual void Update(float dt) override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::SkillSlotHUDController;
	}

private:
	UIImageComponent* m_slotFrames[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	UIImageComponent* m_slotIcons[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
};
