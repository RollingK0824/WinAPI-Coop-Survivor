#include "Engine/Core/pch.h"
#include "SkillChoiceController.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/PrefabManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/UIImageComponent.h"
#include "Engine/Framework/Components/UI/UITextComponent.h"
#include "Engine/Framework/Components/UI/UIButtonComponent.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Game/Manager/InGameManager.h"
#include "Game/Player/Player.h"
#include "Game/Skill/SkillComponent.h"

static ComponentRegistrar<SkillChoiceController> registrar(EngineKey::CustomComponent::SkillChoiceController.data());

SkillChoiceController::SkillChoiceController(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
}

void SkillChoiceController::Start()
{
	ScriptComponent::Start();
}

std::vector<SkillChoiceItem> SkillChoiceController::Generate3Choices(Player* pPlayer)
{
	std::vector<SkillChoiceItem> choices;
	if (!pPlayer) return choices;

	SkillComponent* pSkillComp = pPlayer->gameObject.GetComponent<SkillComponent>();
	if (!pSkillComp) return choices;

	const auto& ownedSkills = pSkillComp->GetSkills();

	std::vector<uint32> allSkillIDs = { 301, 302, 303 };

	bool isSlotMax = (ownedSkills.size() >= MAX_SKILL_SLOTS);
	bool allOwnedMaxed = !ownedSkills.empty();

	std::vector<uint32> eligibleSkillIDs;

	for (const auto& skillInst : ownedSkills)
	{
		if (skillInst.level < MAX_SKILL_LEVEL)
		{
			allOwnedMaxed = false;
			eligibleSkillIDs.push_back(skillInst.pSO->GetSkillID());
		}
	}

	if (allOwnedMaxed && isSlotMax)
	{
		choices.push_back({ ChoiceType::HEAL_PERCENT, 0, L"HP 50% 회복", L"현재 최대 체력의 50%를 즉시 회복합니다." });
		choices.push_back({ ChoiceType::MAX_HP_BOOST, 0, L"최대 HP +20", L"최대 체력을 20 증가시키고 회복합니다." });
		choices.push_back({ ChoiceType::HEAL_FULL, 0, L"체력 완전 회복", L"체력을 100% 최대로 회복합니다." });
		return choices;
	}

	if (!isSlotMax)
	{
		for (uint32 id : allSkillIDs)
		{
			if (!pSkillComp->HasSkill(id))
			{
				eligibleSkillIDs.push_back(id);
			}
		}
	}

	std::mt19937 rng(std::random_device{}());
	std::shuffle(eligibleSkillIDs.begin(), eligibleSkillIDs.end(), rng);

	for (size_t i = 0; i < std::min<size_t>(3, eligibleSkillIDs.size()); ++i)
	{
		uint32 id = eligibleSkillIDs[i];
		bool hasSkill = pSkillComp->HasSkill(id);

		SkillChoiceItem item;
		item.skillID = id;
		item.type = hasSkill ? ChoiceType::SKILL_UPGRADE : ChoiceType::SKILL_NEW;

		int currentLv = hasSkill ? pSkillComp->GetSkillLevel(id) : 0;
		if (hasSkill)
		{
			item.title = L"스킬 레벨업 (Lv." + std::to_wstring(currentLv) + L" -> " + std::to_wstring(currentLv + 1) + L")";
			item.desc = L"소지한 스킬의 위력과 범위를 강화합니다.";
		}
		else
		{
			item.title = L"신규 스킬 획득 (ID: " + std::to_wstring(id) + L")";
			item.desc = L"새로운 공격 기술을 배워 전투력을 보강합니다.";
		}
		choices.push_back(item);
	}

	while (choices.size() < 3)
	{
		choices.push_back({ ChoiceType::HEAL_PERCENT, 0, L"HP 50% 회복", L"체력을 즉시 50% 회복합니다." });
	}

	return choices;
}

void SkillChoiceController::PresentChoices(Player* pPlayer)
{
	if (!pPlayer) return;
	Scene* pScene = gameObject.GetOwnerScene();
	if (!pScene) return;

	InGameManager::GetInstance()->PauseSimulation(true);

	std::vector<SkillChoiceItem> choices = Generate3Choices(pPlayer);

	GameObject* choicePopupObj = PrefabManager::GetInstance()->Instantiate("SkillChoiceUIPrefab", pScene);
	if (!choicePopupObj) return;

	choicePopupObj->transform.SetPosition({ 960.0f, 540.0f });

	auto applyChoiceAndClose = [pScene, choicePopupObj, pPlayer](const SkillChoiceItem& choiceItem) {
		SkillComponent* pSkillComp = pPlayer->gameObject.GetComponent<SkillComponent>();
		if (choiceItem.type == ChoiceType::SKILL_NEW || choiceItem.type == ChoiceType::SKILL_UPGRADE)
		{
			if (pSkillComp)
			{
				pSkillComp->AddSkill(choiceItem.skillID);
			}
		}
		else if (choiceItem.type == ChoiceType::HEAL_PERCENT)
		{
			pPlayer->Heal(pPlayer->GetMaxHP() * 0.5f);
		}
		else if (choiceItem.type == ChoiceType::HEAL_FULL)
		{
			pPlayer->Heal(pPlayer->GetMaxHP());
		}
		else if (choiceItem.type == ChoiceType::MAX_HP_BOOST)
		{
			pPlayer->IncreaseMaxHP(20.0f);
		}

		if (pScene && choicePopupObj)
		{
			pScene->DestroyObjects(choicePopupObj);
		}

		InGameManager::GetInstance()->SendSkillChoiceCompletePacket(choiceItem.skillID, static_cast<uint8>(choiceItem.type));
	};


	UIButtonComponent* cardBtn = choicePopupObj->GetComponent<UIButtonComponent>();
	if (cardBtn)
	{
		if (!choices.empty())
		{
			SkillChoiceItem item = choices[0];
			cardBtn->SetOnClick([applyChoiceAndClose, item]() {
				applyChoiceAndClose(item);
				});
		}
	}

}



