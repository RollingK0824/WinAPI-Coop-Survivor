#include "Engine/Core/pch.h"
#include "Game/Monster/Monster.h"
#include "Engine/Core/Util.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/CircleCollider.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/DataManager.h"
#include "Game/Data/MonsterSO.h"
#include "Game/Player/Player.h"
#include "Game/Monster/MonsterSpawner.h"

static ComponentRegistrar<Monster> registrar(EngineKey::CustomComponent::Monster.data());

Monster::Monster(GameObject* owner, TransformComponent* transform)
	: ScriptComponent(owner, transform)
{
	ExposeAsset<MonsterSO>("MonsterSO", &m_monsterAssetID);
	ExposeVariable("CurrentHP", &m_currentHP);
	ExposeVariable("MaxHP", &m_maxHP);
	ExposeVariable("MoveSpeed", &m_moveSpeed);
	ExposeVariable("AttackDamage", &m_attackDamage);
	ExposeVariable("AttackRange", &m_attackRange);
	ExposeVariable("AttackCooldown", &m_attackCooldown);
}

void Monster::Start()
{
	m_pCollider = gameObject.GetComponent<CircleCollider>();

	if (m_monsterAssetID != 0)
	{
		auto pSO = DataManager::GetInstance()->GetAsset<MonsterSO>(m_monsterAssetID);
		if (pSO)
		{
			Init(0, const_cast<MonsterSO*>(pSO.get()), transform.GetPosition());
		}
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

	m_attackTimer = 0.0f;
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
	if (m_targetPlayer.IsValid() && m_targetPlayer->IsActive())
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
		if (!pObj || !pObj->IsActive()) continue;

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
	if (!m_targetPlayer.IsValid() || !m_targetPlayer->IsActive())
	{
		m_state = EMonsterState::Chase;
		return;
	}

	Vector2 myPos = transform.GetPosition();
	Vector2 targetPos = m_targetPlayer->transform.GetPosition();
	float dist = Vector2::Distance(myPos, targetPos);

	if (dist <= m_attackRange)
	{
		m_state = EMonsterState::Attack;
	}
	else
	{
		m_state = EMonsterState::Chase;
	}
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

	case EMonsterState::Attack:
		if (m_pCollider.IsValid() && b2Body_IsValid(m_pCollider->GetBodyId()))
		{
			b2Body_SetLinearVelocity(m_pCollider->GetBodyId(), { 0.0f, 0.0f });
		}

		m_attackTimer += fixedDt;
		if (m_attackTimer >= m_attackCooldown)
		{
			m_attackTimer = 0.0f;
			OnAttack();
		}
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

void Monster::OnAttack()
{
}

void Monster::OnDie()
{
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
