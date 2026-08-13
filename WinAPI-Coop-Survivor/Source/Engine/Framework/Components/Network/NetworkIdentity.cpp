#include "Engine/Core/pch.h"
#include "NetworkIdentity.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Network/NetworkManager.h"
static ComponentRegistrar<NetworkIdentity>registrar(EngineKey::Component::NetworkIdentity.data());

NetworkIdentity::NetworkIdentity(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner,transform)
{
}

NetworkIdentity::~NetworkIdentity()
{
	if (m_netID != 0)
	{
		NetworkManager::GetInstance()->UnRegisterNetworkObject(m_netID);
	}
}

void NetworkIdentity::SetNetID(uint32 netID)
{
	if (m_netID != 0)
	{
		NetworkManager::GetInstance()->UnRegisterNetworkObject(m_netID);
	}
	m_netID = netID;
	if (m_netID != 0)
	{
		NetworkManager::GetInstance()->RegisterNetworkObject(m_netID, &gameObject);
	}
}

void NetworkIdentity::Start()
{
	if (m_netID != 0)
	{
		NetworkManager::GetInstance()->RegisterNetworkObject(m_netID, &gameObject);
	}
}

void NetworkIdentity::Update(float dt)
{
	if (m_interpData.active)
	{
		m_interpData.elapsed += dt;
	}
}

void NetworkIdentity::SetInterpolationTarget(const Vector2& targetPos, float duration)
{
	// 현재 보간 진행 중이라면 현재 위치를 startPos로 사용
	Vector2 currentPos = targetPos;
	if (m_interpData.active && m_interpData.duration > 0.0f)
	{
		float t = std::clamp(m_interpData.elapsed / m_interpData.duration, 0.0f, 1.0f);
		currentPos = Vector2::Lerp(m_interpData.startPos, m_interpData.targetPos, t);
	}

	m_interpData.startPos  = currentPos;
	m_interpData.targetPos = targetPos;
	m_interpData.elapsed   = 0.0f;
	m_interpData.duration  = duration;
	m_interpData.active    = true;
}

bool NetworkIdentity::GetInterpolatedPosition(Vector2& outPos) const
{
	if (!m_interpData.active) return false;

	float t = (m_interpData.duration > 0.0f)
		? std::clamp(m_interpData.elapsed / m_interpData.duration, 0.0f, 1.0f)
		: 1.0f;
	outPos = Vector2::Lerp(m_interpData.startPos, m_interpData.targetPos, t);
	return true;
}
