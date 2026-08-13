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
#include "Game/Monster/MonsterSpawner.h"
#include "Game/Manager/InGameManager.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Framework/Components/Network/NetworkIdentity.h"
#include "Engine/Framework/Components/Render/SpriteRendererComponent.h"

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

	if (!m_pCollider.IsValid())
	{
		m_pCollider = gameObject.GetComponent<CircleCollider>();
	}
	if (m_pCollider.IsValid())
	{
		m_pCollider->SetFilter(PhysicsLayer::Monster, PhysicsLayer::All);
	}

	if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
	{
		b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f });
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

	if (monsterData)
	{
		m_maxHP = monsterData->GetMaxHP();
		m_currentHP = m_maxHP;
		m_moveSpeed = monsterData->GetMoveSpeed();
		m_attackDamage = monsterData->GetAttackDamage();
		m_expAmount = monsterData->GetExpAmount();
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

	transform.SetPosition(spawnPos);

	if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
	{
		b2Vec2 b2SpawnPos = { PixelToMeter(spawnPos.x), PixelToMeter(spawnPos.y) };
		b2Body_SetTransform(m_pCollider->GetBodyId(), b2SpawnPos, b2Rot_identity);
		b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f });
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

void Monster::FixedUpdate(float fixedDt)
{
	if (m_state == EMonsterState::Dead || !gameObject.IsActive())
		return;

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
				auto pSprite = gameObject.GetComponent<SpriteRendererComponent>();
				if (pSprite)
				{
					pSprite->SetFlip(moveDir.x < 0.0f, false);
				}
			}
		}
		return;
	}

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

		if (pObj->GetComponent<Player>())
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

	// 항상 Chase 상태 유지 (접촉 피해는 Player::OnCollision에서 처리)
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
}

void Monster::TakeDamage(float damage, GameObject* pAttacker)
{
	if (m_state == EMonsterState::Dead) return;

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
	NetRole role = NetworkManager::GetInstance()->GetRole();

	if (role == NetRole::HOST)
	{
		// Host: 로컬 경험치 보석 스폰 및 Client 들에게 MONSTER_KILL 패킷 전송
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
		// 싱글 플레이어: 로컬 경험치 보석 스폰
		if (InGameManager* mgr = InGameManager::GetInstance())
		{
			mgr->SpawnExpGem(transform.GetPosition(), m_expAmount);
		}
	}

	if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
	{
		b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f });
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
