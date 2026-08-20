#include "Engine/Core/pch.h"
#include "SkillSlotHUDController.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/UI/UIImageComponent.h"
#include "Game/Player/Player.h"
#include "Game/Skill/SkillComponent.h"
#include "Game/Skill/SkillSO.h"

static ComponentRegistrar<SkillSlotHUDController> registrar(EngineKey::CustomComponent::SkillSlotHUDController.data());

SkillSlotHUDController::SkillSlotHUDController(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeComponent("Slot Frame 0", &m_slotFrames[0]);
	ExposeComponent("Slot Frame 1", &m_slotFrames[1]);
	ExposeComponent("Slot Frame 2", &m_slotFrames[2]);
	ExposeComponent("Slot Frame 3", &m_slotFrames[3]);
	ExposeComponent("Slot Frame 4", &m_slotFrames[4]);

	ExposeComponent("Slot Icon 0", &m_slotIcons[0]);
	ExposeComponent("Slot Icon 1", &m_slotIcons[1]);
	ExposeComponent("Slot Icon 2", &m_slotIcons[2]);
	ExposeComponent("Slot Icon 3", &m_slotIcons[3]);
	ExposeComponent("Slot Icon 4", &m_slotIcons[4]);
}

void SkillSlotHUDController::Start()
{
	ScriptComponent::Start();

	for (size_t i = 0; i < 5; ++i)
	{
		if (m_slotIcons[i])
		{
			m_slotIcons[i]->gameObject.SetActive(false);
		}
	}
}

void SkillSlotHUDController::Update(float dt)
{
	ScriptComponent::Update(dt);

	uint32 myID = NetworkManager::GetInstance()->GetMyNetID();
	GameObject* myPlayerObj = NetworkManager::GetInstance()->GetNetworkObject(myID != 0 ? myID : 1);
	if (!myPlayerObj) return;

	SkillComponent* pSkillComp = myPlayerObj->GetComponent<SkillComponent>();
	if (!pSkillComp) return;

	const auto& skills = pSkillComp->GetSkills();
	for (size_t i = 0; i < 5; ++i)
	{
		if (!m_slotIcons[i]) continue;

		if (i < skills.size())
		{
			if (!m_slotIcons[i]->gameObject.IsActive())
			{
				m_slotIcons[i]->gameObject.SetActive(true);
			}

			const SkillSO* pSO = skills[i].pSO.get();
			if (pSO)
			{
				std::wstring iconKey = pSO->GetIconKey();
				if (iconKey.empty())
				{
					iconKey = pSO->GetSpriteKey();
				}
				m_slotIcons[i]->SetSpriteKey(iconKey);
			}
		}
		else
		{
			if (m_slotIcons[i]->gameObject.IsActive())
			{
				m_slotIcons[i]->gameObject.SetActive(false);
			}
		}
	}
}
