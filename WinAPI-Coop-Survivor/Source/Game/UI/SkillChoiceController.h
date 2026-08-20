#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/Define.h"

class GameObject;
class Player;
class SkillComponent;
class UIPanelComponent;
class UIImageComponent;
class UITextComponent;
class UIButtonComponent;

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
	int32 nextLevel = 1;
	std::wstring title;
	std::wstring desc;
	std::wstring iconKey;
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
	void OnChoiceSelected(Player* pPlayer, const SkillChoiceItem& choiceItem);
	std::vector<SkillChoiceItem> Generate3Choices(Player* pPlayer);

	static constexpr size_t MAX_SKILL_SLOTS = 5;
	static constexpr int32 MAX_SKILL_LEVEL = 5;

private:
	UIPanelComponent* m_pModalRoot = nullptr;

	struct CardRef
	{
		UIButtonComponent* pBtn = nullptr;
		UIImageComponent* pIcon = nullptr;
		UITextComponent* pNameText = nullptr;
		UITextComponent* pStatusText = nullptr;
		UITextComponent* pDescText = nullptr;
	};

	CardRef m_cards[3];
	bool m_bIsChoiceOpen = false;
};
