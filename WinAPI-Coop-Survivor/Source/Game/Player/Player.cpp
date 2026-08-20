#include "Engine/Core/pch.h"
#include "Player.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Physics/PhysicsManager.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Network/NetworkIdentity.h"
#include "Engine/Framework/Components/Physics/BoxCollider.h"
#include "Engine/Framework/Components/Render/AnimatorComponent.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"
#include "Engine/Framework/Components/UI/UIImageComponent.h"
#include "LocalController.h"
#include "NetworkController.h"
#include "Game/Monster/Monster.h"
#include "Game/Item/ExpGem.h"
#include "Game/Manager/InGameManager.h"
#include "Engine/Manager/PrefabManager.h"
#include "Coffin.h"

static ComponentRegistrar<Player> registrar(EngineKey::CustomComponent::Player.data());

Player::Player(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner, transform)
{
	ExposeVariable("MaxHP", &m_maxHP);
	ExposeVariable("IFrameDuration", &m_iFrameDuration);
}

void Player::SetFacingDirection(const Vector2& dir)
{
	if (dir.LengthSquared() > 0.0001f)
	{
		m_facingDir = dir.GetNormalized();
		if (!m_pSpriteRenderer.IsValid())
		{
			m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
		}
		if (m_pSpriteRenderer.IsValid())
		{
			if (m_facingDir.x < -0.01f)
			{
				m_pSpriteRenderer->SetFlip(true, false);
			}
			else if (m_facingDir.x > 0.01f)
			{
				m_pSpriteRenderer->SetFlip(false, false);
			}
		}
	}
}

void Player::Start()
{
	m_currentHP = m_maxHP;
	m_iFrameTimer = 0.0f;

	m_pCollider = gameObject.GetComponent<ColliderComponent>();
	m_pAnimator = gameObject.GetComponent<AnimatorComponent>();
	m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	m_prevPos = transform.GetPosition();

	if (m_pAnimator.IsValid())
	{
		m_pAnimator->Pause();
	}

	if (m_pCollider.IsValid())
	{
		m_pCollider->m_bIsSensor = false;
		m_pCollider->m_density = 1000.0f;
		m_pCollider->SetFilter(PhysicsLayer::Player, PhysicsLayer::All & ~PhysicsLayer::Player);
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
			b2Body_SetType(m_pCollider->GetBodyId(), b2_dynamicBody);
		}
	}

	CreateHPBarFromPrefab();
}

void Player::Update(float dt)
{
	if (IsDead())
	{
		if (!m_pSpriteRenderer.IsValid())
		{
			m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
		}
		if (m_pSpriteRenderer.IsValid())
		{
			m_pSpriteRenderer->SetOpacity(0.0f);
		}
		UpdateHPBar();
		return;
	}

	if (m_hitFlashTimer > 0.0f)
	{
		m_hitFlashTimer -= dt;
		if (m_hitFlashTimer <= 0.0f)
		{
			m_hitFlashTimer = 0.0f;
			if (!m_pSpriteRenderer.IsValid())
			{
				m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
			}
			if (m_pSpriteRenderer.IsValid())
			{
				m_pSpriteRenderer->SetOpacity(1.0f);
			}
		}
	}

	if (m_iFrameTimer > 0.0f)
	{
		m_iFrameTimer -= dt;
		if (m_iFrameTimer <= 0.0f)
		{
			m_iFrameTimer = 0.0f;
			if (!m_pSpriteRenderer.IsValid())
			{
				m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
			}
			if (m_pSpriteRenderer.IsValid())
			{
				m_pSpriteRenderer->SetOpacity(1.0f);
			}
		}
		else
		{
			if (!m_pSpriteRenderer.IsValid())
			{
				m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
			}
			if (m_pSpriteRenderer.IsValid())
			{
				float blink = (fmod(m_iFrameTimer, 0.2f) < 0.1f) ? 0.35f : 0.9f;
				m_pSpriteRenderer->SetOpacity(blink);
			}
		}
	}

	Vector2 currentPos = transform.GetPosition();
	Vector2 posDelta = currentPos - m_prevPos;
	m_prevPos = currentPos;

	bool isMoving = m_bIsMoving || (posDelta.LengthSquared() > 0.001f);

	if (!m_pAnimator.IsValid())
	{
		m_pAnimator = gameObject.GetComponent<AnimatorComponent>();
	}

	if (m_pAnimator.IsValid())
	{
		if (isMoving && !IsDead())
		{
			if (!m_pAnimator->IsPlaying())
			{
				m_pAnimator->Resume();
			}
		}
		else
		{
			if (m_pAnimator->IsPlaying())
			{
				m_pAnimator->Pause();
			}
		}
	}

	if (!m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	}

	if (m_pSpriteRenderer.IsValid())
	{
		if (m_facingDir.x < -0.01f)
		{
			m_pSpriteRenderer->SetFlip(true, false);
		}
		else if (m_facingDir.x > 0.01f)
		{
			m_pSpriteRenderer->SetFlip(false, false);
		}
	}

	m_bIsMoving = false;

	UpdateHPBar();
	UpdateExpGemMagnet(dt);
}

void Player::UpdateExpGemMagnet(float dt)
{
	if (IsDead()) return;

	InGameManager* mgr = InGameManager::GetInstance();
	if (!mgr || mgr->IsSimulationPaused()) return;

	Vector2 myPos = transform.GetPosition();
	float magnetRange = 50.0f;
	float pickupRange = 10.0f;

	std::vector<ExpGem*> gemsToProcess = mgr->GetActiveGems();

	for (ExpGem* pGem : gemsToProcess)
	{
		if (!pGem || !pGem->gameObject.IsActive()) continue;

		Vector2 gemPos = pGem->transform.GetPosition();
		float dist = Vector2::Distance(myPos, gemPos);

		if (dist <= pickupRange)
		{
			if (NetworkManager::GetInstance()->GetRole() != NetRole::CLIENT)
			{
				mgr->AddTeamExp(static_cast<float>(pGem->GetExpAmount()));
			}

			pGem->Despawn();
			continue;
		}

		if (dist <= magnetRange)
		{
			if (!pGem->HasTargetPlayer())
			{
				pGem->SetTargetPlayer(&gameObject);
			}
			else
			{
				GameObject* currentTarget = pGem->GetTargetPlayer();
				if (currentTarget && currentTarget != &gameObject)
				{
					float currentDist = Vector2::Distance(currentTarget->transform.GetPosition(), gemPos);
					if (dist < currentDist)
					{
						pGem->SetTargetPlayer(&gameObject);
					}
				}
			}
		}
	}
}

void Player::OnDestroy()
{
	ScriptComponent::OnDestroy();

	if (m_pHpBarRootObj.IsValid())
	{
		m_pHpBarRootObj->Destroy();
	}
}

void Player::OnCollision(ColliderComponent* other)
{
	if (!other) return;
	if (m_iFrameTimer > 0.0f) return;
	if (IsDead()) return;

	NetRole role = NetworkManager::GetInstance()->GetRole();
	NetworkIdentity* netId = gameObject.GetComponent<NetworkIdentity>();

	if (role == NetRole::CLIENT && netId && !netId->HasAuthority()) return;

	Monster* pMonster = other->gameObject.GetComponent<Monster>();
	if (!pMonster) return;
	if (pMonster->IsDead()) return;

	TakeDamage(pMonster->GetAttackDamage());
}

void Player::TakeDamage(float damage, GameObject* pAttacker)
{
	if (IsDead()) return;
	if (m_iFrameTimer > 0.0f) return;

	NetRole role = NetworkManager::GetInstance()->GetRole();
	NetworkIdentity* netId = gameObject.GetComponent<NetworkIdentity>();

	if (role == NetRole::CLIENT && netId && !netId->HasAuthority()) return;

	m_currentHP -= damage;
	if (m_currentHP <= 0.0f)
	{
		m_currentHP = 0.0f;

		Scene* pScene = gameObject.GetOwnerScene();
		if (pScene)
		{
			bool coffinExists = false;
			for (GameObject* sceneObj : pScene->GetGameObjects())
			{
				if (sceneObj && sceneObj->IsActive())
				{
					Coffin* c = sceneObj->GetComponent<Coffin>();
					if (c && c->GetTargetPlayer() == this)
					{
						coffinExists = true;
						break;
					}
				}
			}

			if (!coffinExists)
			{
				GameObject* coffinObj = PrefabManager::GetInstance()->Instantiate("Coffin", pScene);
				if (coffinObj)
				{
					coffinObj->transform.SetPosition(transform.GetPosition());
					Coffin* pCoffinComp = coffinObj->GetComponent<Coffin>();
					if (!pCoffinComp)
					{
						pCoffinComp = coffinObj->AddComponent<Coffin>();
					}
					if (pCoffinComp)
					{
						pCoffinComp->SetTargetPlayer(this);
					}
				}
			}
		}

		if (!m_pSpriteRenderer.IsValid())
		{
			m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
		}
		if (m_pSpriteRenderer.IsValid())
		{
			m_pSpriteRenderer->SetOpacity(0.0f);
		}

		UpdateHPBar();
		return;
	}

	m_iFrameTimer = m_iFrameDuration;
	m_hitFlashTimer = 0.15f;

	if (!m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	}
	if (m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer->SetOpacity(0.35f);
	}

	UpdateHPBar();
}

void Player::CreateHPBarFromPrefab()
{
	Scene* pScene = gameObject.GetOwnerScene();
	if (!pScene) return;

	GameObject* pHpBarRoot = PrefabManager::GetInstance()->Instantiate("HpBar", pScene);
	if (!pHpBarRoot) return;

	pHpBarRoot->SetParent(&this->gameObject, false);
	pHpBarRoot->transform.SetLocalPosition(0.0f, 25.0f);

	m_pHpBarRootObj = pHpBarRoot;

	for (GameObject* pChild : pHpBarRoot->GetChildren())
	{
		if (pChild && pChild->GetName() == "HpBar_Filled")
		{
			m_pHpBarFillImg = pChild->GetComponent<UIImageComponent>();
			break;
		}
	}
}

void Player::UpdateHPBar()
{
	if (m_pHpBarRootObj.IsValid())
	{
		m_pHpBarRootObj->SetActive(!IsDead());
	}

	if (m_pHpBarFillImg.IsValid())
	{
		m_pHpBarFillImg->SetFillAmount(GetHPRatio());
	}
}

