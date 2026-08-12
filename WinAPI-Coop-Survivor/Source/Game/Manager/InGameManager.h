#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"

class GameObject;

class InGameManager : public ScriptComponent
{
public:
	CLONEABLE_COMPONENT(InGameManager)

	InGameManager(GameObject* owner, TransformComponent* transform);
	virtual ~InGameManager() override;

	static InGameManager* GetInstance() { return s_instance; }

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::CustomComponent::InGameManager;
	}
	
	virtual void Start() override;
	virtual void Update(float dt) override;
	virtual void OnDestroy() override;

	void SortedPlayerCache();
	const std::vector<GameObject*>& GetPlayers() const { return m_vCachedPlayer; }

	GameObject* SpawnPlayer(uint32 netId, bool isLocal, Vector2 spawnPos);

	void SendClientReadyStatus(bool isReady);
	void SendHostStartSignal();
	bool IsAllClientsReady() const;

	bool IsCountingDown() const { return m_bIsCountDown; }
	float GetCountdownTimer() const { return m_countdownTimer; }
	bool IsGameStarted() const { return m_bIsGameStarted; }
	void SetGameStarted(bool started) { m_bIsGameStarted = started; }

	void SetOnCountdownTickCallback(std::function<void(float)> cb) { m_onCountdownTick = cb; }
	void SetOnGameStartedCallback(std::function<void()> cb) { m_onGameStarted = cb; }
	void SetOnReadyStatusChangedCallback(std::function<void(bool)> cb) { m_onReadyStatusChanged = cb; }

private:
	static InGameManager* s_instance;
	std::unordered_map<uint32, GameObject*> m_playerObjects;
	std::vector<GameObject*> m_vCachedPlayer;

	std::unordered_map<uint32, bool> m_clientReadyMap;
	bool m_bIsMyReady = false;

	bool m_bIsCountDown = false;
	bool m_bIsGameStarted = false;
	float m_countdownTimer = 0.0f;

	std::function<void(float)> m_onCountdownTick;
	std::function<void()> m_onGameStarted;
	std::function<void(bool)> m_onReadyStatusChanged;
};

