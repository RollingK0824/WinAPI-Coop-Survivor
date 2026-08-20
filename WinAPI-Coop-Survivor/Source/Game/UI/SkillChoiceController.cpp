#include "Engine/Core/pch.h"
#include "SkillChoiceController.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/DataManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/UIPanelComponent.h"
#include "Engine/Framework/Components/UI/UIImageComponent.h"
#include "Engine/Framework/Components/UI/UITextComponent.h"
#include "Engine/Framework/Components/UI/UIButtonComponent.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Game/Manager/InGameManager.h"
#include "Game/Player/Player.h"
#include "Game/Skill/SkillComponent.h"
#include "Game/Skill/SkillSO.h"
#include "Engine/Core/Util.h"
#include <random>
#include <algorithm>

static ComponentRegistrar<SkillChoiceController> registrar(EngineKey::CustomComponent::SkillChoiceController.data());

SkillChoiceController::SkillChoiceController(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeComponent("Modal Root", &m_pModalRoot);

	ExposeComponent("Card 0 Button", &m_cards[0].pBtn);
	ExposeComponent("Card 0 Icon", &m_cards[0].pIcon);
	ExposeComponent("Card 0 Name Text", &m_cards[0].pNameText);
	ExposeComponent("Card 0 Status Text", &m_cards[0].pStatusText);
	ExposeComponent("Card 0 Desc Text", &m_cards[0].pDescText);

	ExposeComponent("Card 1 Button", &m_cards[1].pBtn);
	ExposeComponent("Card 1 Icon", &m_cards[1].pIcon);
	ExposeComponent("Card 1 Name Text", &m_cards[1].pNameText);
	ExposeComponent("Card 1 Status Text", &m_cards[1].pStatusText);
	ExposeComponent("Card 1 Desc Text", &m_cards[1].pDescText);

	ExposeComponent("Card 2 Button", &m_cards[2].pBtn);
	ExposeComponent("Card 2 Icon", &m_cards[2].pIcon);
	ExposeComponent("Card 2 Name Text", &m_cards[2].pNameText);
	ExposeComponent("Card 2 Status Text", &m_cards[2].pStatusText);
	ExposeComponent("Card 2 Desc Text", &m_cards[2].pDescText);
}

void SkillChoiceController::Start()
{
	ScriptComponent::Start();

	if (m_pModalRoot)
	{
		m_pModalRoot->gameObject.SetActive(false);
	}
}

std::vector<SkillChoiceItem> SkillChoiceController::Generate3Choices(Player* pPlayer)
{
	std::vector<SkillChoiceItem> choices;
	if (!pPlayer) return choices;

	SkillComponent* pSkillComp = pPlayer->gameObject.GetComponent<SkillComponent>();
	if (!pSkillComp) return choices;

	const auto& ownedSkills = pSkillComp->GetSkills();

	std::vector<uint32> allSkillIDs;
	for (const auto& [id, pSO] : DataManager::GetInstance()->GetAllAssets())
	{
		if (pSO && pSO->GetSOTypeName() == "SkillSO")
		{
			allSkillIDs.push_back(id);
		}
	}
	if (allSkillIDs.empty())
	{
		allSkillIDs = { 301, 302, 303, 304, 305 };
	}
	std::sort(allSkillIDs.begin(), allSkillIDs.end());

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
		choices.push_back({ ChoiceType::HEAL_PERCENT, 0, 1, L"HP 50% 회복", L"현재 최대 체력의 50%를 즉시 회복합니다.", L"UI/frameB_gold.png" });
		choices.push_back({ ChoiceType::MAX_HP_BOOST, 0, 1, L"최대 HP +20", L"최대 체력을 20 증가시키고 회복합니다.", L"UI/frameB_gold.png" });
		choices.push_back({ ChoiceType::HEAL_FULL, 0, 1, L"체력 완전 회복", L"체력을 100% 최대로 회복합니다.", L"UI/frameB_gold.png" });
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

		auto pSkillSO = DataManager::GetInstance()->GetSkillSO(id);
		std::wstring skillNameW = pSkillSO ? Utf8ToWide(pSkillSO->GetSkillName()) : (L"Skill " + std::to_wstring(id));

		int currentLv = hasSkill ? pSkillComp->GetSkillLevel(id) : 0;
		int nextLv = currentLv + 1;
		item.nextLevel = nextLv;

		std::wstring descW = L"스킬 능력을 강화합니다.";
		if (pSkillSO)
		{
			const auto& lvlData = pSkillSO->GetLevelData(nextLv);
			if (!lvlData.description.empty())
			{
				descW = Utf8ToWide(lvlData.description);
			}

			item.iconKey = pSkillSO->GetIconKey();
			if (item.iconKey.empty())
			{
				item.iconKey = pSkillSO->GetSpriteKey();
			}
		}

		item.title = skillNameW;
		item.desc = descW;
		choices.push_back(item);
	}

	while (choices.size() < 3)
	{
		choices.push_back({ ChoiceType::HEAL_PERCENT, 0, 1, L"HP 50% 회복", L"체력을 즉시 50% 회복합니다.", L"UI/frameB_gold.png" });
	}

	return choices;
}

void SkillChoiceController::PresentChoices(Player* pPlayer)
{
	if (!pPlayer) return;
	if (!m_pModalRoot) return;
	if (m_bIsChoiceOpen) return;

	InGameManager::GetInstance()->PauseSimulation(true);
	m_bIsChoiceOpen = true;

	std::vector<SkillChoiceItem> choices = Generate3Choices(pPlayer);

	for (size_t i = 0; i < 3; ++i)
	{
		if (i < choices.size())
		{
			const SkillChoiceItem& item = choices[i];

			if (m_cards[i].pBtn)
			{
				m_cards[i].pBtn->gameObject.SetActive(true);
				m_cards[i].pBtn->SetOnClick([this, pPlayer, item]() {
					OnChoiceSelected(pPlayer, item);
				});
			}

			if (m_cards[i].pIcon)
			{
				m_cards[i].pIcon->SetSpriteKey(item.iconKey);
			}

			if (m_cards[i].pNameText)
			{
				m_cards[i].pNameText->SetText(item.title);
			}

			if (m_cards[i].pStatusText)
			{
				if (item.type == ChoiceType::SKILL_NEW)
				{
					m_cards[i].pStatusText->SetText(L"신규!");
					m_cards[i].pStatusText->SetColor(D2D1::ColorF(1.0f, 0.95f, 0.15f, 1.0f));
				}
				else if (item.type == ChoiceType::SKILL_UPGRADE)
				{
					m_cards[i].pStatusText->SetText(L"레벨:" + std::to_wstring(item.nextLevel));
					m_cards[i].pStatusText->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f));
				}
				else
				{
					m_cards[i].pStatusText->SetText(L"회복");
					m_cards[i].pStatusText->SetColor(D2D1::ColorF(0.2f, 1.0f, 0.4f, 1.0f));
				}
			}

			if (m_cards[i].pDescText)
			{
				m_cards[i].pDescText->SetText(item.desc);
			}
		}
		else
		{
			if (m_cards[i].pBtn)
			{
				m_cards[i].pBtn->gameObject.SetActive(false);
			}
		}
	}

	m_pModalRoot->gameObject.SetActive(true);
}

void SkillChoiceController::OnChoiceSelected(Player* pPlayer, const SkillChoiceItem& choiceItem)
{
	if (!m_bIsChoiceOpen) return;
	m_bIsChoiceOpen = false;

	SkillComponent* pSkillComp = pPlayer ? pPlayer->gameObject.GetComponent<SkillComponent>() : nullptr;
	if (choiceItem.type == ChoiceType::SKILL_NEW || choiceItem.type == ChoiceType::SKILL_UPGRADE)
	{
		if (pSkillComp)
		{
			pSkillComp->AddSkill(choiceItem.skillID);
		}
	}
	else if (choiceItem.type == ChoiceType::HEAL_PERCENT)
	{
		if (pPlayer) pPlayer->Heal(pPlayer->GetMaxHP() * 0.5f);
	}
	else if (choiceItem.type == ChoiceType::HEAL_FULL)
	{
		if (pPlayer) pPlayer->Heal(pPlayer->GetMaxHP());
	}
	else if (choiceItem.type == ChoiceType::MAX_HP_BOOST)
	{
		if (pPlayer) pPlayer->IncreaseMaxHP(20.0f);
	}

	if (m_pModalRoot)
	{
		m_pModalRoot->gameObject.SetActive(false);
	}

	InGameManager::GetInstance()->SendSkillChoiceCompletePacket(choiceItem.skillID, static_cast<uint8>(choiceItem.type));
}
