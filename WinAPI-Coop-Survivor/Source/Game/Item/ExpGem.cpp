#include "Engine/Core/pch.h"
#include "ExpGem.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/UI/UIImageComponent.h"
#include "Game/Manager/InGameManager.h"

static ComponentRegistrar<ExpGem> registrar(EngineKey::CustomComponent::ExpGem.data());

ExpGem::ExpGem(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeVariable("ExpAmount", &m_expAmount);
	ExposeVariable("FlySpeed", &m_flySpeed);
}

void ExpGem::Start()
{
	ScriptComponent::Start();

	UIImageComponent* pImg = gameObject.GetComponent<UIImageComponent>();
	if (!pImg)
	{
		pImg = gameObject.AddComponent<UIImageComponent>();
	}
	if (pImg)
	{
		pImg->SetIsUI(false);
		pImg->SetSize({ 12.0f, 12.0f });
		pImg->SetColor(D2D1::ColorF(0.1f, 0.85f, 1.0f, 1.0f));
		pImg->SetZOrder(150);
	}
}

void ExpGem::OnEnable()
{
	ScriptComponent::OnEnable();
	m_targetPlayer = nullptr;

	if (InGameManager* mgr = InGameManager::GetInstance())
	{
		mgr->RegisterGem(this);
	}
}

void ExpGem::OnDisable()
{
	ScriptComponent::OnDisable();
	m_targetPlayer = nullptr;

	if (InGameManager* mgr = InGameManager::GetInstance())
	{
		mgr->UnregisterGem(this);
	}
}

void ExpGem::Init(int32 expAmount)
{
	m_expAmount = expAmount;
	m_targetPlayer = nullptr;
}

void ExpGem::Despawn()
{
	m_targetPlayer = nullptr;
	PoolManager::GetInstance()->Despawn("ExpGemPrefab", &gameObject);
}

void ExpGem::Update(float dt)
{
	if (!gameObject.IsActive()) return;

	InGameManager* mgr = InGameManager::GetInstance();
	if (!mgr || mgr->IsSimulationPaused()) return;

	// Player 주체가 타깃으로 지정한 경우 플레이어 방향으로 이동만 수행 (획득/경험치 연산은 Player가 담당)
	if (m_targetPlayer.IsValid())
	{
		Vector2 myPos = transform.GetPosition();
		Vector2 playerPos = m_targetPlayer->transform.GetPosition();
		Vector2 dir = (playerPos - myPos).GetNormalized();
		transform.SetPosition(myPos + dir * m_flySpeed * dt);
	}
}
