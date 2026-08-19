#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class GameObject;
class Player;
class SkillComponent;

enum class ChoiceType {
	SKILL_NEW,
	SKILL_UPGRADE,
	HEAL_PERCENT,
	HEAL_FULL,
	MAX_HP_BOOST
};

struct SkillChoiceItem {
	ChoiceType type = ChoiceType::SKILL_NEW;
	uint32 skillID = 0;
	std::wstring title;
	std::wstring desc;
};

class SkillChoiceController : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(SkillChoiceController)

	SkillChoiceController(GameObject* owner, TransformComponent* transform);
	virtual ~SkillChoiceController() override = default;

	virtual void Start() override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::SkillChoiceController;
	}

	void PresentChoices(Player* pPlayer);

private:
	std::vector<SkillChoiceItem> Generate3Choices(Player* pPlayer);

	static constexpr size_t MAX_SKILL_SLOTS = 4;
	static constexpr int32 MAX_SKILL_LEVEL = 5;
};
