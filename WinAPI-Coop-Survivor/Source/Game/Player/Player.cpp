#include "Engine/Core/pch.h"
#include "Player.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Network/NetworkIdentity.h"
#include "Engine/Framework/Components/Physics/BoxCollider.h"
#include "Engine/Framework/Components/UI/UIImageComponent.h"
#include "Engine/Physics/PhysicsManager.h"
#include "LocalController.h"
#include "NetworkController.h"
#include "Game/Monster/Monster.h"

static ComponentRegistrar<Player> registrar(EngineKey::CustomComponent::Player.data());

Player::Player(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner, transform)
{
	ExposeVariable("MaxHP", &m_maxHP);
	ExposeVariable("IFrameDuration", const_cast<float*>(&k_iFrameDuration));
}

void Player::Start()
{
	m_currentHP = m_maxHP;
	m_iFrameTimer = 0.0f;

	m_pCollider = gameObject.GetComponent<ColliderComponent>();

	// Sensor(Trigger)로 설정: 몬스터와 물리 밀림 없이 OnCollision 이벤트만 수신
	if (m_pCollider.IsValid())
	{
		m_pCollider->m_bIsSensor = true;
		m_pCollider->SetFilter(PhysicsLayer::Player, PhysicsLayer::All);
		m_pCollider->RebuildShape();
	}

	NetworkIdentity* netIdentity = gameObject.GetComponent<NetworkIdentity>();
	if (!netIdentity) return;

	if (netIdentity->HasAuthority())
	{
		gameObject.AddComponent<LocalController>();
	}
	else
	{
		gameObject.AddComponent<NetworkController>(netIdentity->GetNetID());

		if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
		{
			b2Body_SetType(m_pCollider->GetBodyId(), b2_kinematicBody);
		}
	}

	CreateTestHPBar();
}

void Player::Update(float dt)
{
	if (m_iFrameTimer > 0.0f)
	{
		m_iFrameTimer -= dt;
	}
	else if (!IsDead())
	{
		// 몬스터 무리 접촉 피해 검사 (무적 시간 0.3초마다 정확히 1회만 피해 적용)
		Vector2 myPos = transform.GetPosition();
		float hitRadius = 24.0f; // 플레이어 피격 판정 반경

		auto colliders = PhysicsManager::GetInstance()->OverlapAABB(myPos, hitRadius, PhysicsLayer::Monster);
		for (auto* pCol : colliders)
		{
			if (!pCol || !pCol->IsEnabled() || !pCol->gameObject.IsActive()) continue;

			Monster* pMonster = pCol->gameObject.GetComponent<Monster>();
			if (pMonster && !pMonster->IsDead())
			{
				TakeDamage(pMonster->GetAttackDamage());
				break; // 1회 피격 후 무적시간(iFrame) 재설정되므로 바로 탈출
			}
		}
	}

	UpdateHPBar();
}

void Player::OnDestroy()
{
	ScriptComponent::OnDestroy();

	Scene* pScene = gameObject.GetOwnerScene();
	if (pScene)
	{
		if (m_pHpBarBgObj.IsValid())
		{
			pScene->DestroyObjects(m_pHpBarBgObj.Get());
		}
		if (m_pHpBarFillObj.IsValid())
		{
			pScene->DestroyObjects(m_pHpBarFillObj.Get());
		}
	}
}

void Player::OnCollision(ColliderComponent* other)
{
	if (!other) return;
	if (m_iFrameTimer > 0.0f) return;
	if (IsDead()) return;

	Monster* pMonster = other->gameObject.GetComponent<Monster>();
	if (!pMonster) return;
	if (pMonster->IsDead()) return;

	TakeDamage(pMonster->GetAttackDamage());
}

void Player::TakeDamage(float damage, GameObject* pAttacker)
{
	if (IsDead()) return;

	m_currentHP -= damage;
	if (m_currentHP <= 0.0f)
	{
		m_currentHP = 0.0f;
	}

	m_iFrameTimer = k_iFrameDuration;
}

void Player::CreateTestHPBar()
{
	Scene* pScene = gameObject.GetOwnerScene();
	if (!pScene) return;

	Vector2 playerPos = transform.GetPosition();
	Vector2 hpBarPos = { playerPos.x, playerPos.y - 45.0f };

	// 1. HPBar Background GameObject (월드 스페이스 UI)
	m_pHpBarBgObj = pScene->CreateGameObject("Test_HPBar_BG");
	if (m_pHpBarBgObj.IsValid())
	{
		m_pHpBarBgObj->transform.SetPosition(hpBarPos);
		UIImageComponent* pBgImg = m_pHpBarBgObj->AddComponent<UIImageComponent>();
		if (pBgImg)
		{
			pBgImg->SetIsUI(false); // 월드 좌표 따라가도록 설정
			pBgImg->SetSize({ 50.0f, 6.0f });
			pBgImg->SetColor(D2D1::ColorF(0.2f, 0.2f, 0.2f, 0.8f));
			pBgImg->SetZOrder(500);
		}
	}

	// 2. HPBar Fill GameObject (월드 스페이스 UI + FillAmount 연동)
	m_pHpBarFillObj = pScene->CreateGameObject("Test_HPBar_Fill");
	if (m_pHpBarFillObj.IsValid())
	{
		m_pHpBarFillObj->transform.SetPosition(hpBarPos);
		m_pHpBarFillImg = m_pHpBarFillObj->AddComponent<UIImageComponent>();
		if (m_pHpBarFillImg.IsValid())
		{
			m_pHpBarFillImg->SetIsUI(false); // 월드 좌표 따라가도록 설정
			m_pHpBarFillImg->SetSize({ 50.0f, 6.0f });
			m_pHpBarFillImg->SetColor(D2D1::ColorF(0.9f, 0.1f, 0.1f, 1.0f));
			m_pHpBarFillImg->SetFillAmount(1.0f);
			m_pHpBarFillImg->SetZOrder(501);
		}
	}
}

void Player::UpdateHPBar()
{
	Vector2 playerPos = transform.GetPosition();
	Vector2 hpBarPos = { playerPos.x, playerPos.y - 45.0f };

	if (m_pHpBarBgObj.IsValid())
	{
		m_pHpBarBgObj->transform.SetPosition(hpBarPos);
		m_pHpBarBgObj->SetActive(!IsDead());
	}

	if (m_pHpBarFillObj.IsValid())
	{
		m_pHpBarFillObj->transform.SetPosition(hpBarPos);
		m_pHpBarFillObj->SetActive(!IsDead());
	}

	if (m_pHpBarFillImg.IsValid())
	{
		m_pHpBarFillImg->SetFillAmount(GetHPRatio());
	}
}
