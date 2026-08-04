#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class NetworkIdentity : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(NetworkIdentity)

	NetworkIdentity(GameObject* owner, TransformComponent* transform);
	virtual ~NetworkIdentity() override = default;

	void SetNetID(unsigned int netID) { m_netID = netID; }
	unsigned int GetNetID() const { return m_netID; }

	void SetLocalPlayer(bool isLocal) { m_bIsLocalPlayer = isLocal; }
	bool IsLocalPlayer() const { return m_bIsLocalPlayer; }

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::NetworkIdentity.data();
	}
private:
	unsigned int m_netID = 0;
	bool m_bIsLocalPlayer = false;
};