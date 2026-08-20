#include "Engine/Core/pch.h"
#include "Game/Monster/Monster.h"
#include "Engine/Core/Util.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/CircleCollider.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/DataManager.h"
#include "Game/Monster/MonsterSO.h"
#include "Game/Player/Player.h"
#include "Game/Player/Coffin.h"
#include "Game/Monster/MonsterSpawner.h"
#include "Game/Manager/InGameManager.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Framework/Components/Network/NetworkIdentity.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"
#include "Engine/Framework/Components/Render/AnimatorComponent.h"
#include "Engine/Manager/ResourceManager.h"

static ComponentRegistrar<Monster> registrar(EngineKey::CustomComponent::Monster.data());

Monster::Monster(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeAsset<MonsterSO>("MonsterSO", &m_monsterAssetID);
	ExposeVariable("CurrentHP", &m_currentHP);
	ExposeVariable("MaxHP", &m_maxHP);
	ExposeVariable("MoveSpeed", &m_moveSpeed);
	ExposeVariable("AttackDamage", &m_attackDamage);
}

void Monster::Start()
{
	m_pCollider = gameObject.GetComponent<CircleCollider>();
	m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	if (m_pCollider.IsValid())
	{
		m_pCollider->SetFilter(PhysicsLayer::Monster, PhysicsLayer::All);
	}
}

void Monster::OnEnable()
{
	m_state = EMonsterState::Chase;
	m_targetPlayer = nullptr;
	m_targetSearchTimer = 0.0f;

	m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	if (!m_pCollider.IsValid())
	{
		m_pCollider = gameObject.GetComponent<CircleCollider>();
	}
	if (m_pCollider.IsValid())
	{
		m_pCollider->SetFilter(PhysicsLayer::Monster, PhysicsLayer::All);

		if (b2Body_IsValid(m_pCollider->GetBodyId()))
		{
			NetRole role = NetworkManager::GetInstance()->GetRole();
			if (role == NetRole::CLIENT)
			{
				b2Body_SetType(m_pCollider->GetBodyId(), b2_kinematicBody);
				m_pCollider->m_bIsSensor = true;
				m_pCollider->RebuildShape();
			}
			else
			{
				b2Body_SetType(m_pCollider->GetBodyId(), b2_dynamicBody);
				m_pCollider->m_bIsSensor = false;
				m_pCollider->RebuildShape();
			}
			b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f });
		}
	}
}

void Monster::OnDisable()
{
	m_targetPlayer = nullptr;
	if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
	{
		b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f });
	}
}

void Monster::Init(uint32 spawnSeqId, MonsterSO* monsterData, const Vector2& spawnPos, MonsterSpawner* spawner)
{
	m_spawnSeqID = spawnSeqId;
	m_state = EMonsterState::Chase;
	m_pSpawner = spawner;

	if (!m_pCollider.IsValid())
	{
		m_pCollider = gameObject.GetComponent<CircleCollider>();
	}
	if (!m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	}

	if (monsterData)
	{
		m_pMonsterSO = monsterData;
		m_monsterAssetID = monsterData->GetAssetID();
		m_maxHP = monsterData->GetMaxHP();
		m_currentHP = m_maxHP;
		m_moveSpeed = monsterData->GetMoveSpeed();
		m_attackDamage = monsterData->GetAttackDamage();
		m_expAmount = monsterData->GetExpAmount();

		if (m_pCollider.IsValid())
		{
			m_pCollider->SetRadius(monsterData->GetColliderRadius());
		}

		AnimatorComponent* pAnim = gameObject.GetComponent<AnimatorComponent>();
		if (pAnim)
		{
			pAnim->SetPlaySpeed(0.5f);
			pAnim->SetOnAnimationFinished(nullptr);
		}

		const std::string& animKey = monsterData->GetAnimClipKey();
		if (!animKey.empty() && pAnim)
		{
			std::wstring wAnimKey(animKey.begin(), animKey.end());
			const AnimationClip* pClip = ResourceManager::GetInstance()->GetAnimationClip(wAnimKey);
			if (pClip)
			{
				pAnim->AddClip(*pClip);
			}

			const std::string& dieKey = monsterData->GetDieClipKey();
			if (!dieKey.empty())
			{
				std::wstring wDieKey(dieKey.begin(), dieKey.end());
				const AnimationClip* pDieClip = ResourceManager::GetInstance()->GetAnimationClip(wDieKey);
				if (pDieClip)
				{
					pAnim->AddClip(*pDieClip);
				}
			}

			pAnim->Play(wAnimKey, true);
		}
		else if (m_pSpriteRenderer.IsValid() && !monsterData->GetSpriteKey().empty())
		{
			if (pAnim)
			{
				pAnim->Stop();
			}
			m_pSpriteRenderer->SetSpriteKey(monsterData->GetSpriteKey());
		}
	}
	else
	{
		m_maxHP = 100.0f;
		m_currentHP = 100.0f;
		m_moveSpeed = 120.0f;
		m_attackDamage = 10.0f;
		m_expAmount = 10;
	}

	m_targetSearchTimer = 0.0f;
	m_targetPlayer = nullptr;
	m_hitFlashTimer = 0.0f;

	if (m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer->SetOpacity(1.0f);
	}

	transform.SetPosition(spawnPos);

	if (m_pCollider.IsValid())
	{
		m_pCollider->SetFilter(PhysicsLayer::Monster, PhysicsLayer::All);

		if (b2Body_IsValid(m_pCollider->GetBodyId()))
		{
			NetRole role = NetworkManager::GetInstance()->GetRole();
			if (role == NetRole::CLIENT)
			{
				b2Body_SetType(m_pCollider->GetBodyId(), b2_kinematicBody);
				m_pCollider->m_bIsSensor = true;
				m_pCollider->RebuildShape();
			}
			else
			{
				b2Body_SetType(m_pCollider->GetBodyId(), b2_dynamicBody);
				m_pCollider->m_bIsSensor = false;
				m_pCollider->RebuildShape();
			}

			b2Vec2 b2SpawnPos = { PixelToMeter(spawnPos.x), PixelToMeter(spawnPos.y) };
			b2Body_SetTransform(m_pCollider->GetBodyId(), b2SpawnPos, b2Rot_identity);
			b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f });
		}
	}
}

void Monster::SetTargetPlayer(GameObject* pTargetObj)
{
	m_targetPlayer = pTargetObj;
}

void Monster::SetSpawner(MonsterSpawner* spawner)
{
	m_pSpawner = spawner;
}

void Monster::Update(float dt)
{
	if (m_state == EMonsterState::Dead || !gameObject.IsActive())
		return;

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

	NetRole role = NetworkManager::GetInstance()->GetRole();
	if (role == NetRole::CLIENT)
	{
		Vector2 lerpPos;
		NetworkIdentity* netId = gameObject.GetComponent<NetworkIdentity>();
		if (netId && netId->GetInterpolatedPosition(lerpPos))
		{
			Vector2 currentPos = transform.GetPosition();
			Vector2 moveDir = lerpPos - currentPos;

			transform.SetPosition(lerpPos);

			if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
			{
				b2Vec2 b2Pos = { PixelToMeter(lerpPos.x), PixelToMeter(lerpPos.y) };
				b2Body_SetTransform(m_pCollider->GetBodyId(), b2Pos, b2Rot_identity);
				b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f });
			}

			if (std::abs(moveDir.x) > 0.01f)
			{
				if (!m_pSpriteRenderer.IsValid())
				{
					m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
				}
				if (m_pSpriteRenderer.IsValid())
				{
					m_pSpriteRenderer->SetFlip(moveDir.x > 0.0f, false);
				}
			}
		}
	}
}

void Monster::FixedUpdate(float fixedDt)
{
	if (m_state == EMonsterState::Dead || !gameObject.IsActive())
		return;

	NetRole role = NetworkManager::GetInstance()->GetRole();
	if (role == NetRole::CLIENT)
		return;

	UpdateTargetSearch(fixedDt);
	UpdateAI(fixedDt);
	UpdateBehaviour(fixedDt);
}

void Monster::UpdateTargetSearch(float fixedDt)
{
	m_targetSearchTimer += fixedDt;
	if (m_targetSearchTimer < m_targetSearchInterval)
		return;

	m_targetSearchTimer = 0.0f;

	Scene* pScene = gameObject.GetOwnerScene();
	if (!pScene) return;

	float currentTargetDist = (std::numeric_limits<float>::max)();
	if (m_targetPlayer.IsValid() && !m_targetPlayer->IsDead() && m_targetPlayer->IsActive())
	{
		currentTargetDist = Vector2::Distance(transform.GetPosition(), m_targetPlayer->transform.GetPosition());
	}
	else
	{
		m_targetPlayer = nullptr;
	}

	GameObject* pBestPlayer = m_targetPlayer.Get();
	float bestDist = currentTargetDist;

	const auto& sceneObjects = pScene->GetGameObjects();
	for (const auto& pObj : sceneObjects)
	{
		if (!pObj || pObj->IsDead() || !pObj->IsActive()) continue;

		Player* pPlayer = pObj->GetComponent<Player>();
		Coffin* pCoffin = pObj->GetComponent<Coffin>();
		if ((pPlayer && !pPlayer->IsDead()) || pCoffin)
		{
			float dist = Vector2::Distance(transform.GetPosition(), pObj->transform.GetPosition());
			
			if (dist < bestDist - m_targetMargin || (!m_targetPlayer.IsValid() && dist < bestDist))
			{
				bestDist = dist;
				pBestPlayer = pObj;
			}
		}
	}

	m_targetPlayer = pBestPlayer;
}

void Monster::UpdateAI(float fixedDt)
{
	if (!m_targetPlayer.IsValid() || m_targetPlayer->IsDead() || !m_targetPlayer->IsActive())
	{
		m_state = EMonsterState::Chase;
		return;
	}

	m_state = EMonsterState::Chase;
}

void Monster::UpdateBehaviour(float fixedDt)
{
	switch (m_state)
	{
	case EMonsterState::Spawn:
		break;

	case EMonsterState::Chase:
		MoveTowardsTarget(fixedDt);
		break;

	case EMonsterState::Dead:
		if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
		{
			b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f });
		}
		break;
	}
}

void Monster::MoveTowardsTarget(float fixedDt)
{
	if (!m_targetPlayer.IsValid()) return;

	Vector2 myPos = transform.GetPosition();
	Vector2 targetPos = m_targetPlayer->transform.GetPosition();
	Vector2 dir = (targetPos - myPos).GetNormalized();

	if (!m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	}
	if (m_pSpriteRenderer.IsValid() && std::abs(dir.x) > 0.01f)
	{
		m_pSpriteRenderer->SetFlip(dir.x > 0.0f, false);
	}

	if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
	{
		Vector2 targetVelocity = dir * m_moveSpeed;
		b2Vec2 b2Velocity = { PixelToMeter(targetVelocity.x), PixelToMeter(targetVelocity.y) };
		b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), b2Velocity);
	}
	else
	{
		transform.SetPosition(transform.GetPosition() + dir * m_moveSpeed * fixedDt);
	}

	float distToTarget = Vector2::Distance(myPos, targetPos);
	if (distToTarget <= 28.0f)
	{
		Player* pPlayer = m_targetPlayer->GetComponent<Player>();
		if (pPlayer && !pPlayer->IsDead())
		{
			pPlayer->TakeDamage(m_attackDamage, &gameObject);
		}
	}
}

void Monster::TakeDamage(float damage, GameObject* pAttacker)
{
	if (m_state == EMonsterState::Dead) return;

	m_hitFlashTimer = 0.12f;
	if (!m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer = gameObject.GetComponent<SpriteRendererComponent>();
	}
	if (m_pSpriteRenderer.IsValid())
	{
		m_pSpriteRenderer->SetOpacity(0.35f);
	}

	if (InGameManager* pMgr = InGameManager::GetInstance())
	{
		pMgr->SpawnDamageText(static_cast<int>(damage + 0.5f), transform.GetPosition(), damage >= 50.0f);
	}

	NetRole role = NetworkManager::GetInstance()->GetRole();
	if (role == NetRole::CLIENT) return;

	m_currentHP -= damage;
	if (m_currentHP <= 0.0f)
	{
		m_currentHP = 0.0f;
		m_state = EMonsterState::Dead;
		OnDie();
	}
}

void Monster::OnDie()
{
	m_state = EMonsterState::Dead;

	if (m_pCollider.IsValid())
	{
		m_pCollider->SetFilter(PhysicsLayer::None, PhysicsLayer::None);
		if (b2Body_IsValid(m_pCollider->GetBodyId()))
		{
			b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f });
		}
	}

	AnimatorComponent* pAnim = gameObject.GetComponent<AnimatorComponent>();
	if (m_pMonsterSO.IsValid() && !m_pMonsterSO->GetDieClipKey().empty() && pAnim)
	{
		pAnim->SetPlaySpeed(2.0f);
		std::wstring wDieKey(m_pMonsterSO->GetDieClipKey().begin(), m_pMonsterSO->GetDieClipKey().end());
		pAnim->Play(wDieKey, true);
		pAnim->SetOnAnimationFinished([this](const std::wstring& clipName) {
			DespawnSelf();
		});
	}
	else
	{
		DespawnSelf();
	}
}

void Monster::ClientDie()
{
	if (m_state == EMonsterState::Dead) return;
	m_state = EMonsterState::Dead;

	if (m_pCollider.IsValid())
	{
		m_pCollider->SetFilter(PhysicsLayer::None, PhysicsLayer::None);
		if (b2Body_IsValid(m_pCollider->GetBodyId()))
		{
			b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f });
		}
	}

	AnimatorComponent* pAnim = gameObject.GetComponent<AnimatorComponent>();
	if (m_pMonsterSO.IsValid() && !m_pMonsterSO->GetDieClipKey().empty() && pAnim)
	{
		pAnim->SetPlaySpeed(2.0f);
		std::wstring wDieKey(m_pMonsterSO->GetDieClipKey().begin(), m_pMonsterSO->GetDieClipKey().end());
		pAnim->Play(wDieKey, true);
		pAnim->SetOnAnimationFinished([this](const std::wstring& clipName) {
			DespawnSelf();
		});
	}
	else
	{
		DespawnSelf();
	}
}

void Monster::DespawnSelf()
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

	AnimatorComponent* pAnim = gameObject.GetComponent<AnimatorComponent>();
	if (pAnim)
	{
		pAnim->SetPlaySpeed(1.0f);
		pAnim->SetOnAnimationFinished(nullptr);
	}

	NetRole role = NetworkManager::GetInstance()->GetRole();
	if (role == NetRole::HOST)
	{
		if (InGameManager* mgr = InGameManager::GetInstance())
		{
			mgr->SpawnExpGem(transform.GetPosition(), m_expAmount);
		}

		MonsterKillPacket killPacket{};
		killPacket.header.type = PacketType::MONSTER_KILL;
		killPacket.header.size = sizeof(MonsterKillPacket);
		killPacket.monsterNetID = m_netID;
		killPacket.dropItemPos = transform.GetPosition();

		NetworkManager::GetInstance()->SendReliablePacket(&killPacket, sizeof(MonsterKillPacket));
	}
	else if (role == NetRole::NONE)
	{
		if (InGameManager* mgr = InGameManager::GetInstance())
		{
			mgr->SpawnExpGem(transform.GetPosition(), m_expAmount);
		}
	}

	if (m_pSpawner.IsValid())
	{
		m_pSpawner->DespawnMonster(&gameObject);
	}
	else
	{
		gameObject.SetActive(false);
	}
}
