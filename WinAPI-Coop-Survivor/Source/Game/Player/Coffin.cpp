#include "Engine/Core/pch.h"
#include "Coffin.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"
#include "Game/Player/Player.h"

static ComponentRegistrar<Coffin> registrar(EngineKey::CustomComponent::Coffin.data());

Coffin::Coffin(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeVariable("MaxRezTime", &m_maxRezTime);
}

void Coffin::Start()
{
	ScriptComponent::Start();

	m_rezTimer = 0.0f;

	auto* pRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	if (pRenderer)
	{
		pRenderer->SetSpriteKey(L"Coffin/Coffin.png");
		pRenderer->SetZOrder(100);
		pRenderer->SetOpacity(1.0f);
	}

	for (GameObject* child : gameObject.GetChildren())
	{
		if (child && child->GetName() == "ProgressBar")
		{
			m_pBarSpriteRenderer = child->GetComponent<SpriteRendererComponent>();
			if (m_pBarSpriteRenderer.IsValid())
			{
				m_pBarSpriteRenderer->SetZOrder(102);
				m_pBarSpriteRenderer->SetOpacity(1.0f);
			}
			break;
		}
	}

	UpdateProgressBar();
}

void Coffin::Update(float dt)
{
	ScriptComponent::Update(dt);

	if (!m_pBarSpriteRenderer.IsValid())
	{
		for (GameObject* child : gameObject.GetChildren())
		{
			if (child && child->GetName() == "ProgressBar")
			{
				m_pBarSpriteRenderer = child->GetComponent<SpriteRendererComponent>();
				if (m_pBarSpriteRenderer.IsValid())
				{
					m_pBarSpriteRenderer->SetZOrder(102);
					m_pBarSpriteRenderer->SetOpacity(1.0f);
				}
				break;
			}
		}
	}

	m_rezTimer += dt;
	if (m_rezTimer >= m_maxRezTime)
	{
		m_rezTimer = m_maxRezTime;
		UpdateProgressBar();

		if (m_pTargetPlayer.IsValid())
		{
			m_pTargetPlayer->transform.SetPosition(transform.GetPosition());
			m_pTargetPlayer->gameObject.SetActive(true);
			m_pTargetPlayer->Heal(m_pTargetPlayer->GetMaxHP());
			m_pTargetPlayer->SetInvincible(5.0f);
		}

		gameObject.Destroy();
		return;
	}

	UpdateProgressBar();
}

void Coffin::UpdateProgressBar()
{
	if (!m_pBarSpriteRenderer.IsValid()) return;

	float ratio = m_rezTimer / m_maxRezTime;
	if (ratio < 0.0f) ratio = 0.0f;
	if (ratio > 1.0f) ratio = 1.0f;

	int frameIdx = static_cast<int>(ratio * 25.0f);
	if (frameIdx < 0) frameIdx = 0;
	if (frameIdx > 25) frameIdx = 25;

	std::wstring spriteKey = L"CoffinRezBarSpritesheet/CoffinRezBarSpritesheet_" + std::to_wstring(frameIdx) + L".png";
	m_pBarSpriteRenderer->SetSpriteKey(spriteKey);
}
