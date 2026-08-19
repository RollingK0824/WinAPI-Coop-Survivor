#include "Engine/Core/pch.h"
#include "NetworkIdentity.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
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

void NetworkIdentity::OnDisable()
{
	ResetInterpolation(transform.GetPosition());
}

void NetworkIdentity::Update(float dt)
{
	if (m_interpData.active)
	{
		m_interpData.elapsed += dt;
	}
}

void NetworkIdentity::ResetInterpolation(const Vector2& pos)
{
	m_interpData.startPos  = pos;
	m_interpData.targetPos = pos;
	m_interpData.elapsed   = 0.0f;
	m_interpData.duration  = 0.0f;
	m_interpData.active    = false;
}

void NetworkIdentity::SetInterpolationTarget(const Vector2& targetPos, float duration)
{
	Vector2 currentPos = transform.GetPosition();
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
