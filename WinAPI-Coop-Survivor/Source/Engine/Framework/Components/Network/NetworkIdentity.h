#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class NetworkIdentity : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(NetworkIdentity)

	NetworkIdentity(GameObject* owner, TransformComponent* transform);
	virtual ~NetworkIdentity() override;

	virtual void Start() override;
	virtual void OnDisable() override;
	virtual void Update(float dt) override;

	void SetNetID(uint32 netID);
	uint32 GetNetID() const { return m_netID; }

	void SetLocalPlayer(bool isLocal) { m_bIsLocalPlayer = isLocal; }
	bool IsLocalPlayer() const { return m_bIsLocalPlayer; }
	bool HasAuthority() const { return m_bIsLocalPlayer; }

	void ResetInterpolation(const Vector2& pos);
	void SetInterpolationTarget(const Vector2& targetPos, float duration = 0.0166f);
	bool GetInterpolatedPosition(Vector2& outPos) const;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::NetworkIdentity.data();
	}

private:
	uint32 m_netID = 0;
	bool m_bIsLocalPlayer = false;

	struct InterpolationData {
		Vector2 startPos  { 0.0f, 0.0f };
		Vector2 targetPos { 0.0f, 0.0f };
		float   elapsed   = 0.0f;
		float   duration  = 0.0f;
		bool    active    = false;
	} m_interpData;
};