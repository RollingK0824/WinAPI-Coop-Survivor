#pragma once
#include "Engine/Framework/Components/Core/ScriptComponent.h"
#include "Engine/Core/ObserverPtr.h"

class ExpGem;
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
	void SetOnTeamLevelUpCallback(std::function<void(int32)> cb) { m_onTeamLevelUp = cb; }
	void SetOnExpChangedCallback(std::function<void(float, float)> cb) { m_onExpChanged = cb; }

	// 팀 경험치 & 레벨 시스템
	void AddTeamExp(float amount);
	void PauseSimulation(bool pause);
	bool IsSimulationPaused() const { return m_bIsSimulationPaused; }

	int32 GetTeamLevel() const { return m_teamLevel; }
	float GetTeamExp() const { return m_teamExp; }
	float GetTeamMaxExp() const { return m_teamMaxExp; }
	float GetTeamExpRatio() const { return (m_teamMaxExp > 0.0f) ? (m_teamExp / m_teamMaxExp) : 0.0f; }

	void SpawnExpGem(Vector2 pos, int32 expAmount);

	// 활성 경험치 보석 등록 및 관리 (Player 주체 자력 흡수를 위한 최적화)
	void RegisterGem(ExpGem* gem);
	void UnregisterGem(ExpGem* gem);
	const std::vector<ExpGem*>& GetActiveGems() const { return m_activeGems; }

private:
	void CreateTeamExpBarUI();
	void UpdateTeamExpBarUI();

private:
	static InGameManager* s_instance;
	std::unordered_map<uint32, GameObject*> m_playerObjects;
	std::vector<GameObject*> m_vCachedPlayer;

	std::unordered_map<uint32, bool> m_clientReadyMap;
	bool m_bIsMyReady = false;

	bool m_bIsCountDown = false;
	bool m_bIsGameStarted = false;
	float m_countdownTimer = 0.0f;

	// 팀 공용 레벨 & 경험치
	int32 m_teamLevel = 1;
	float m_teamExp = 0.0f;
	float m_teamMaxExp = 100.0f;
	bool m_bIsSimulationPaused = false;

	// 활성 보석 관리 리스트
	std::vector<ExpGem*> m_activeGems;

	// 팀 경험치 바 UI (UIImage, UIText 조립)
	ObserverPtr<GameObject> m_pExpBarBgObj;
	ObserverPtr<GameObject> m_pExpBarFillObj;
	ObserverPtr<class UIImageComponent> m_pExpBarFillImg;
	ObserverPtr<GameObject> m_pExpTextObj;
	ObserverPtr<class UITextComponent> m_pExpTextComp;

	std::function<void(float)> m_onCountdownTick;
	std::function<void()> m_onGameStarted;
	std::function<void(bool)> m_onReadyStatusChanged;
	std::function<void(int32)> m_onTeamLevelUp;
	std::function<void(float, float)> m_onExpChanged;
};

