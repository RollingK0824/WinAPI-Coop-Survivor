#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class GameObject;

class InGameManager : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(InGameManager)

	InGameManager(GameObject* owner, TransformComponent* transform);
	virtual ~InGameManager() override = default;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::InGameManager;
	}

	virtual void Start() override;
	virtual void Update(float dt) override;

	GameObject* SpawnPlayer(uint32 netId, bool isLocal, Vector2 spawnPos);

private:
	std::unordered_map<uint32, GameObject*> m_playerObjects;
};
