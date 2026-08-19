#include "Engine/Core/pch.h"
#include "InGameManager.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Manager/TimeManager.h"
#include "Engine/Manager/RandomManager.h"
#include "Engine/Manager/PrefabManager.h"
#include "Engine/Core/ObjectPool.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Network/NetworkIdentity.h"
#include "Engine/Framework/Components/Physics/ColliderComponent.h"
#include "Engine/Manager/DebugManager.h"
#include "Game/Player/Player.h"
#include "Game/Monster/Monster.h"
#include "Game/Monster/MonsterSpawner.h"
#include "Game/Skill/SkillComponent.h"
#include "Game/Item/ExpGem.h"
#include "Engine/Framework/Components/UI/UIImageComponent.h"
#include "Engine/Framework/Components/UI/UITextComponent.h"
#include "Game/Player/NetworkController.h"
#include "Game/UI/SkillChoiceController.h"
#include "Game/Manager/GameEvents.h"
#include "Engine/Core/EventBus.h"


InGameManager* InGameManager::s_instance = nullptr;

static ComponentRegistrar<InGameManager> registrar(EngineKey::CustomComponent::InGameManager.data());

InGameManager::InGameManager(GameObject* owner, TransformComponent* transform) : ScriptComponent(owner, transform)
{
	s_instance = this;

	ExposeVariable("TeamLevel", &m_teamLevel);
	ExposeVariable("TeamExp", &m_teamExp);
	ExposeVariable("TeamMaxExp", &m_teamMaxExp);
	ExposeVariable("IsSimulationPaused", &m_bIsSimulationPaused);
	ExposeComponent("EXP Bar Fill Image", &m_pExpBarFillImg);
	ExposeComponent("EXP Text Component", &m_pExpTextComp);
}

InGameManager::~InGameManager()
{
	if (s_instance == this) s_instance = nullptr;
}

void InGameManager::OnDestroy()
{
	ScriptComponent::OnDestroy();
	if (s_instance == this) s_instance = nullptr;
}


void InGameManager::Start()
{
	TimeManager::GetInstance()->SetPaused(false);
	m_countdownTimer = 3.0f;
	m_bIsCountDown = false;
	m_bIsGameStarted = (NetworkManager::GetInstance()->GetRole() == NetRole::NONE);

	m_teamLevel = 1;
	m_teamExp = 0.0f;
	m_teamMaxExp = 100.0f;
	m_bIsSimulationPaused = false;

	CreateTeamExpBarUI();

	Scene* pScene = gameObject.GetOwnerScene();
	if (pScene)
	{
		DebugManager::GetInstance()->CreateDebugUIOverlay(pScene);

		auto initSkillPool = [pScene](const std::string& key, size_t cap) {
			PoolManager::GetInstance()->CreatePool<GameObject>(
				key,
				[key, pScene]() { return PrefabManager::GetInstance()->Instantiate(key, pScene); },
				[](GameObject* obj) { if (obj) obj->SetActive(true); },
				[](GameObject* obj) { if (obj) obj->SetActive(false); },
				nullptr,
				cap, 300
			);
		};

		initSkillPool("GenericProjectilePrefab", 300);
		initSkillPool("GenericAuraPrefab", 20);
		initSkillPool("GenericAoEPrefab", 50);

		// ExpGem 오브젝트 풀 초기화 (미등록 시 런타임 자동 생성 폴백 백업)
		PoolManager::GetInstance()->CreatePool<GameObject>(
			"ExpGemPrefab",
			[pScene]() {
				GameObject* pObj = PrefabManager::GetInstance()->Instantiate("ExpGemPrefab", pScene);
				if (!pObj)
				{
					pObj = pScene->CreateGameObject("ExpGem");
					pObj->AddComponent<ExpGem>();
					UIImageComponent* pImg = pObj->AddComponent<UIImageComponent>();
					if (pImg)
					{
						pImg->SetIsUI(false);
						pImg->SetSize({ 12.0f, 12.0f });
						pImg->SetColor(D2D1::ColorF(0.1f, 0.85f, 1.0f, 1.0f));
						pImg->SetZOrder(150);
					}
				}
				return pObj;
			},
			[](GameObject* obj) { if (obj) obj->SetActive(true); },
			[](GameObject* obj) { if (obj) obj->SetActive(false); },
			nullptr,
			300, 1000
		);
	}

	if (!gameObject.GetComponent<MonsterSpawner>())
	{
		gameObject.AddComponent<MonsterSpawner>();
	}

	if (!gameObject.GetComponent<SkillChoiceController>())
	{
		gameObject.AddComponent<SkillChoiceController>();
	}


	NetworkManager* net = NetworkManager::GetInstance();
	uint32 myNetID = net->GetMyNetID();

	if (myNetID != 0)
	{
		float startX = (static_cast<float>(myNetID) - 1.0f) * 120.0f;
		SpawnPlayer(myNetID, true, { startX, 0.0f });
	}

	if (net->GetRole() == NetRole::CLIENT)
	{
		net->RegisterPacketHandler(PacketType::HOST_WELCOME,
			[this](const PacketHeader* packet, const sockaddr_in& sender) {
				auto welcome = reinterpret_cast<const WelcomePacket*>(packet);
				float startX = (static_cast<float>(welcome->assignedNetID) - 1.0f) * 120.0f;
				this->SpawnPlayer(welcome->assignedNetID, true, { startX, 0.0f });
			});

		net->RegisterPacketHandler(PacketType::MONSTER_SNAPSHOT,
			[this](const PacketHeader* packet, const sockaddr_in& sender) {
				int size = packet->size;
				if (size < static_cast<int>(sizeof(MonsterSnapshotPacket))) return;

				const MonsterSnapshotPacket* snapshot = reinterpret_cast<const MonsterSnapshotPacket*>(packet);

				size_t expectedSize = sizeof(MonsterSnapshotPacket);
				if (snapshot->monsterCount > 1)
					expectedSize += (snapshot->monsterCount - 1) * sizeof(MonsterSnapshotData);
				if (size < static_cast<int>(expectedSize)) return;

				// MonsterSpawner 탐색
				MonsterSpawner* spawner = nullptr;
				Scene* pScene = gameObject.GetOwnerScene();
				if (pScene)
				{
					for (auto* obj : pScene->GetGameObjects())
					{
						if (obj && obj->IsActive())
						{
							spawner = obj->GetComponent<MonsterSpawner>();
							if (spawner) break;
						}
					}
				}

				for (uint16 i = 0; i < snapshot->monsterCount; ++i)
				{
					uint16 monsterNetID = snapshot->monsters[i].monsterNetID;
					Vector2 targetPos   = snapshot->monsters[i].pos;

					Monster* pMonster = nullptr;
					if (spawner)
					{
						pMonster = spawner->GetMonsterByNetID(monsterNetID);
						if (!pMonster)
						{
							pMonster = spawner->SpawnMonsterClient(monsterNetID, targetPos);
						}
						else
						{
							// 컬링 후 재진입 등 거리 차이가 큰 경우 즉시 위치 세팅(Snap)하여 대각선 고속 이동/텔레포트 방지
							float dist = Vector2::Distance(pMonster->transform.GetPosition(), targetPos);
							if (dist > 150.0f)
							{
								pMonster->transform.SetPosition(targetPos);
							}
						}
					}

					if (pMonster)
					{
						NetworkIdentity* netId = pMonster->gameObject.GetComponent<NetworkIdentity>();
						if (netId) netId->SetInterpolationTarget(targetPos, 0.08f);
					}
				}
			});

		net->RegisterPacketHandler(PacketType::MONSTER_KILL,
			[this](const PacketHeader* packet, const sockaddr_in& sender) {
				auto killPkt = reinterpret_cast<const MonsterKillPacket*>(packet);
				this->SpawnExpGem(killPkt->dropItemPos, 10);

				// Client 몬스터 Despawn 처리
				Scene* pScene = gameObject.GetOwnerScene();
				if (pScene)
				{
					for (auto* obj : pScene->GetGameObjects())
					{
						if (obj && obj->IsActive())
						{
							auto* spawner = obj->GetComponent<MonsterSpawner>();
							if (spawner)
							{
								spawner->DespawnMonsterByNetID(killPkt->monsterNetID);
								break;
							}
						}
					}
				}
			});

		net->RegisterPacketHandler(PacketType::TEAM_EXP_SYNC,
			[this](const PacketHeader* packet, const sockaddr_in& sender) {
				auto syncPkt = reinterpret_cast<const TeamExpSyncPacket*>(packet);
				bool leveledUp = (syncPkt->teamLevel > this->m_teamLevel);
				this->m_teamLevel = syncPkt->teamLevel;
				this->m_teamExp = syncPkt->teamExp;
				this->m_teamMaxExp = syncPkt->teamMaxExp;

				if (leveledUp && this->m_onTeamLevelUp)
				{
					this->m_onTeamLevelUp(this->m_teamLevel);
				}

				this->UpdateTeamExpBarUI();
			});

		net->RegisterPacketHandler(PacketType::SIMULATION_RESUME_SIGNAL,
			[this](const PacketHeader* packet, const sockaddr_in& sender) {
				this->PauseSimulation(false);
			});
	}

	if (net->GetRole() == NetRole::HOST)
	{
		for (const auto& [clientNetID, clientInfo] : net->GetConnectedClients())
		{
			float startX = (static_cast<float>(clientNetID) - 1.0f) * 120.0f;
			SpawnPlayer(clientNetID, false, { startX, 0.0f });
		}

		net->RegisterPacketHandler(PacketType::SKILL_CHOICE_COMPLETE,
			[this](const PacketHeader* packet, const sockaddr_in& sender) {
				auto choicePkt = reinterpret_cast<const SkillChoiceCompletePacket*>(packet);
				this->NotifySkillChoiceComplete(choicePkt->playerNetID, choicePkt->chosenSkillID, choicePkt->choiceType);
			});

		net->RegisterPacketHandler(PacketType::CLIENT_READY_REQ,

			[this](const PacketHeader* packet, const sockaddr_in& sender) {
				auto readyPkt = reinterpret_cast<const ClientReadyReqPacket*>(packet);
				this->m_clientReadyMap[readyPkt->netId] = readyPkt->isReady;

				if (this->m_onReadyStatusChanged)
				{
					this->m_onReadyStatusChanged(this->IsAllClientsReady());
				}
			});

		net->RegisterPacketHandler(PacketType::PLAYER_INPUT,
			[this](const PacketHeader* packet, const sockaddr_in& sender) {
				auto inputPkt = reinterpret_cast<const PlayerInputPacket*>(packet);
				uint32 clientNetID = inputPkt->netID;

				if (clientNetID != 0 && !NetworkManager::GetInstance()->GetNetworkObject(clientNetID))
				{
					this->SpawnPlayer(clientNetID, false, inputPkt->pos);
				}

				GameObject* obj = NetworkManager::GetInstance()->GetNetworkObject(clientNetID);
				if (obj) {
					NetworkIdentity* netId = obj->GetComponent<NetworkIdentity>();
					if (netId) netId->SetInterpolationTarget({ inputPkt->pos.x, inputPkt->pos.y }, 0.033f);

					NetworkController* netCtrl = obj->GetComponent<NetworkController>();
					if (netCtrl) netCtrl->SetVelocity(inputPkt->vel);
				}
			});
	}

	net->RegisterPacketHandler(PacketType::GAME_START_SIGNAL,
		[this](const PacketHeader* packet, const sockaddr_in& sender) {
			auto startPkt = reinterpret_cast<const GameStartSignalPacket*>(packet);

			RandomManager::GetInstance()->SetSharedSeed(startPkt->randomSeed);

			float pingMs = NetworkManager::GetInstance()->GetPing();
			float latencySec = (pingMs * 0.5f) / 1000.0f;
			float finalCountdown = startPkt->countdown;
			if (finalCountdown > latencySec)
			{
				finalCountdown -= latencySec;
			}

			this->m_countdownTimer = finalCountdown;
			this->m_bIsCountDown = true;
			this->SortedPlayerCache();
		});

	net->RegisterPacketHandler(PacketType::ENTITY_STATE_SYNC,
		[this](const PacketHeader* packet, const sockaddr_in& sender) {
			auto syncPkt = reinterpret_cast<const EntityStateSyncPacket*>(packet);
			uint32 myID = NetworkManager::GetInstance()->GetMyNetID();

			for (int i = 0; i < syncPkt->entityCount; ++i)
			{
				const EntitySyncData& entity = syncPkt->entities[i];

				GameObject* obj = NetworkManager::GetInstance()->GetNetworkObject(entity.netID);
				if (!obj)
				{
					bool isLocal = (entity.netID == myID);
					this->SpawnPlayer(entity.netID, isLocal, entity.pos);
					obj = NetworkManager::GetInstance()->GetNetworkObject(entity.netID);
				}

				if (obj)
				{
					if (auto* player = obj->GetComponent<Player>())
					{
						player->SyncHP(entity.hp);
					}

					if (entity.netID != myID)
					{
						if (auto* netId = obj->GetComponent<NetworkIdentity>())
						{
							netId->SetInterpolationTarget({ entity.pos.x, entity.pos.y }, 0.033f);
						}
					}
				}
			}
		});

	net->RegisterPacketHandler(PacketType::CLIENT_DISCONN,
		[this](const PacketHeader* packet, const sockaddr_in& sender) {
			auto disconnPkt = reinterpret_cast<const ClientDisconnPacket*>(packet);
			uint32 deadNetID = disconnPkt->disconnectedNetID;

			auto iter = m_playerObjects.find(deadNetID);
			if (iter != m_playerObjects.end())
			{
				Scene* pScene = gameObject.GetOwnerScene();
				if (pScene && iter->second)
				{
					pScene->DestroyObjects(iter->second);
				}
				m_playerObjects.erase(iter);
			}
		});
}

void InGameManager::Update(float dt)
{
	if (m_bIsCountDown)
	{
		float unscaledDt = TimeManager::GetInstance()->GetUnscaledDeltaTime();
		m_countdownTimer -= unscaledDt;

		if (m_onCountdownTick)
		{
			m_onCountdownTick(m_countdownTimer);
		}

		if (m_countdownTimer <= 0.0f)
		{
			m_countdownTimer = 0.0f;
			m_bIsCountDown = false;
			m_bIsGameStarted = true;

			this->SortedPlayerCache();

			MonsterSpawner* spawner = gameObject.GetComponent<MonsterSpawner>();
			if (spawner)
			{
				spawner->StartSpawning();
			}

			TimeManager::GetInstance()->SetPaused(false);

			if (m_onGameStarted)
			{
				m_onGameStarted();
			}
		}
	}

	NetworkManager* net = NetworkManager::GetInstance();
	if (!net->IsConnected()) return;
}

void InGameManager::FixedUpdate(float fixedDt)
{
	NetworkManager* net = NetworkManager::GetInstance();
	if (!net->IsConnected() || net->GetRole() != NetRole::HOST) return;

	BroadcastPlayerEntityState();
}

void InGameManager::BroadcastPlayerEntityState()
{
	NetworkManager* net = NetworkManager::GetInstance();
	Scene* scene = gameObject.GetOwnerScene();
	if (!scene) return;

	EntityStateSyncPacket syncPacket;
	syncPacket.header.type = PacketType::ENTITY_STATE_SYNC;
	syncPacket.header.size = sizeof(EntityStateSyncPacket);
	syncPacket.header.tick = net->GetCurrentTick();
	syncPacket.entityCount = 0;

	for (auto* obj : scene->GetGameObjects())
	{
		if (obj && obj->IsActive())
		{
			NetworkIdentity* netIdComp = obj->GetComponent<NetworkIdentity>();
			// 몬스터 제외, 플레이어 객체만 ENTITY_STATE_SYNC로 동기화 (NetID < 1000)
			if (netIdComp && netIdComp->GetNetID() > 0 && netIdComp->GetNetID() < 1000)
			{
				int idx = syncPacket.entityCount;
				if (idx >= 32) break;

				syncPacket.entities[idx].netID = netIdComp->GetNetID();

				ColliderComponent* pCollider = obj->GetComponent<ColliderComponent>();
				if (pCollider && b2Body_IsValid(pCollider->GetBodyId()))
				{
					b2Vec2 pos = b2Body_GetPosition(pCollider->GetBodyId());
					b2Vec2 vel = b2Body_GetLinearVelocity(pCollider->GetBodyId());
					float angle = b2Rot_GetAngle(b2Body_GetRotation(pCollider->GetBodyId()));

					syncPacket.entities[idx].pos = Vector2(MeterToPixel(pos.x), MeterToPixel(pos.y));
					syncPacket.entities[idx].vel = Vector2(MeterToPixel(vel.x), MeterToPixel(vel.y));
					syncPacket.entities[idx].angle = angle;
				}
				else
				{
					TransformComponent* transform = &obj->transform;
					syncPacket.entities[idx].pos = transform->GetPosition();
					syncPacket.entities[idx].vel = Vector2(0.0f, 0.0f);
					syncPacket.entities[idx].angle = transform->GetRotation().angle;
				}

				Player* pPlayer = obj->GetComponent<Player>();
				syncPacket.entities[idx].hp = pPlayer ? pPlayer->GetCurrentHP() : 100.0f;

				syncPacket.entityCount++;
			}
		}
	}

	if (syncPacket.entityCount > 0)
	{
		int packetSize = sizeof(PacketHeader) + sizeof(int) + sizeof(EntitySyncData) * syncPacket.entityCount;
		net->SendPacket(&syncPacket, packetSize);
	}
}

void InGameManager::SortedPlayerCache()
{
	m_vCachedPlayer.clear();

	std::vector<std::pair<uint32, GameObject*>> tempPairs;
	for (const auto& [netId, pPlayerObj] : m_playerObjects)
	{
		if (pPlayerObj && pPlayerObj->IsActive())
		{
			tempPairs.push_back({ netId,pPlayerObj });
		}
	}

	std::sort(tempPairs.begin(), tempPairs.end(), [](const auto& a, const auto& b)
		{
			return a.first < b.first;
		});

	for (const auto& pair : tempPairs)
	{
		m_vCachedPlayer.push_back(pair.second);
	}
}

GameObject* InGameManager::SpawnPlayer(uint32 netId, bool isLocal, Vector2 spawnPos)
{
	if (GameObject* existingObj = NetworkManager::GetInstance()->GetNetworkObject(netId))
	{
		return existingObj;
	}

	Scene* pScene = gameObject.GetOwnerScene();
	if (!pScene) return nullptr;

	GameObject* pPlayerObj = PrefabManager::GetInstance()->Instantiate("PlayerPrefab", pScene);
	if (!pPlayerObj)
	{
		return nullptr;
	}

	// 스폰 좌표가 0,0 기본값이면 NetID에 따라 가로 120px 간격으로 일렬 스폰
	if (spawnPos.x == 0.0f && spawnPos.y == 0.0f)
	{
		spawnPos.x = (static_cast<float>(netId) - 1.0f) * 120.0f;
	}

	pPlayerObj->transform.SetPosition(spawnPos);

	// Box2D 물리 강체 좌표도 스폰 위치로 즉시 동기화 (0,0 겹침 방지)
	ColliderComponent* pCollider = pPlayerObj->GetComponent<ColliderComponent>();
	if (pCollider && b2Body_IsValid(pCollider->GetBodyId()))
	{
		b2Vec2 b2Pos = { PixelToMeter(spawnPos.x), PixelToMeter(spawnPos.y) };
		b2Body_SetTransform(pCollider->GetBodyId(), b2Pos, b2Rot_identity);
	}

	NetworkIdentity* netIdentity = pPlayerObj->GetComponent<NetworkIdentity>();
	if (!netIdentity)
	{
		netIdentity = pPlayerObj->AddComponent<NetworkIdentity>();
	}
	netIdentity->SetNetID(netId);
	netIdentity->SetLocalPlayer(isLocal);

	// SkillComponent 가져오기 (없으면 추가)
	SkillComponent* pSkillComp = pPlayerObj->GetComponent<SkillComponent>();
	if (!pSkillComp)
	{
		pSkillComp = pPlayerObj->AddComponent<SkillComponent>();
	}

	m_playerObjects[netId] = pPlayerObj;

	return pPlayerObj;
}

void InGameManager::SendClientReadyStatus(bool isReady)
{
	m_bIsMyReady = isReady;
	uint32 myNetID = NetworkManager::GetInstance()->GetMyNetID();

	ClientReadyReqPacket pk{};
	pk.header.type = PacketType::CLIENT_READY_REQ;
	pk.header.size = sizeof(ClientReadyReqPacket);
	pk.netId = myNetID;
	pk.isReady = isReady;

	NetworkManager::GetInstance()->SendPacket(&pk, sizeof(pk));
}

void InGameManager::SendHostStartSignal()
{
	if (!IsAllClientsReady())return;

	uint32 newSeed = RandomManager::GetInstance()->GenerateNewSeed();

	GameStartSignalPacket pk{};
	pk.header.type = PacketType::GAME_START_SIGNAL;
	pk.header.size = sizeof(GameStartSignalPacket);
	pk.randomSeed = newSeed;
	pk.countdown = 3.0f;

	NetworkManager::GetInstance()->SendPacket(&pk, sizeof(pk));

	this->m_countdownTimer = 3.0f;
	this->m_bIsCountDown = true;

	this->SortedPlayerCache();
}

bool InGameManager::IsAllClientsReady() const
{
	NetworkManager* net = NetworkManager::GetInstance();
	const auto& clients = net->GetConnectedClients();

	if (clients.empty())return true;

	for (const auto& [netID, info] : clients)
	{
		auto iter = m_clientReadyMap.find(netID);
		if (iter == m_clientReadyMap.end() || !iter->second)
		{
			return false;
		}
	}
	return true;
}

void InGameManager::AddTeamExp(float amount)
{
	m_teamExp += amount;

	if (m_onExpChanged)
	{
		m_onExpChanged(m_teamExp, m_teamMaxExp);
	}

	// 팀 경험치가 목표치를 달성할 때까지 반복 레벨업 처리 (경험치 이월 지원)
	while (m_teamExp >= m_teamMaxExp)
	{
		m_teamExp -= m_teamMaxExp;
		m_teamLevel++;
		m_teamMaxExp *= 1.25f; // 다음 레벨업 필요 경험치 25% 증가

		if (m_onTeamLevelUp)
		{
			m_onTeamLevelUp(m_teamLevel);
		}

		// 레벨업 3중 1택 스킬 선택 UI 표시
		if (auto* choiceCtrl = gameObject.GetComponent<SkillChoiceController>())
		{
			uint32 myID = NetworkManager::GetInstance()->GetMyNetID();
			GameObject* myPlayerObj = NetworkManager::GetInstance()->GetNetworkObject(myID != 0 ? myID : 1);
			if (myPlayerObj)
			{
				Player* pPlayer = myPlayerObj->GetComponent<Player>();
				if (pPlayer)
				{
					choiceCtrl->PresentChoices(pPlayer);
				}
			}
		}
	}

	// Host 권한 기반 팀 레벨/경험치 상태 패킷 동기화 전송
	if (NetworkManager::GetInstance()->GetRole() == NetRole::HOST)
	{
		TeamExpSyncPacket syncPkt{};
		syncPkt.header.type = PacketType::TEAM_EXP_SYNC;
		syncPkt.header.size = sizeof(TeamExpSyncPacket);
		syncPkt.teamLevel = m_teamLevel;
		syncPkt.teamExp = m_teamExp;
		syncPkt.teamMaxExp = m_teamMaxExp;

		NetworkManager::GetInstance()->SendReliablePacket(&syncPkt, sizeof(syncPkt));
	}

	UpdateTeamExpBarUI();
}

void InGameManager::PauseSimulation(bool pause)
{
	m_bIsSimulationPaused = pause;
	TimeManager::GetInstance()->SetPaused(pause);
}

void InGameManager::SpawnExpGem(Vector2 pos, int32 expAmount)
{
	GameObject* pGemObj = PoolManager::GetInstance()->Spawn<GameObject>("ExpGemPrefab");
	if (pGemObj)
	{
		pGemObj->transform.SetPosition(pos);
		ExpGem* pGem = pGemObj->GetComponent<ExpGem>();
		if (!pGem)
		{
			pGem = pGemObj->AddComponent<ExpGem>();
		}
		if (pGem)
		{
			pGem->Init(expAmount);
		}
		pGemObj->SetActive(true);
	}
}

void InGameManager::CreateTeamExpBarUI()
{
	Scene* pScene = gameObject.GetOwnerScene();
	if (!pScene) return;

	// Scene 파일(InGameScene.scene)에 사전 등록된 UI 오브젝트 바인딩
	if (!m_pExpBarFillImg)
	{
		if (GameObject* fillObj = pScene->FindGameObjectByName("TeamEXPBar_Fill"))
		{
			m_pExpBarFillImg = fillObj->GetComponent<UIImageComponent>();
		}
	}

	if (!m_pExpTextComp)
	{
		if (GameObject* textObj = pScene->FindGameObjectByName("TeamEXP_Text"))
		{
			m_pExpTextComp = textObj->GetComponent<UITextComponent>();
		}
	}

	UpdateTeamExpBarUI();
}

void InGameManager::UpdateTeamExpBarUI()
{
	if (m_pExpBarFillImg)
	{
		m_pExpBarFillImg->SetFillAmount(GetTeamExpRatio());
	}

	if (m_pExpTextComp)
	{
		std::wstring text = L"LV." + std::to_wstring(m_teamLevel) + L"  ("
			+ std::to_wstring(static_cast<int>(m_teamExp)) + L" / "
			+ std::to_wstring(static_cast<int>(m_teamMaxExp)) + L")";
		m_pExpTextComp->SetText(text);
	}
}


void InGameManager::RegisterGem(ExpGem* gem)
{
	if (!gem) return;
	auto it = std::find(m_activeGems.begin(), m_activeGems.end(), gem);
	if (it == m_activeGems.end())
	{
		m_activeGems.push_back(gem);
	}
}

void InGameManager::UnregisterGem(ExpGem* gem)
{
	if (!gem) return;
	auto it = std::find(m_activeGems.begin(), m_activeGems.end(), gem);
	if (it != m_activeGems.end())
	{
		m_activeGems.erase(it);
	}
}

void InGameManager::SendSkillChoiceCompletePacket(uint32 chosenSkillID, uint8 choiceType)
{
	uint32 myNetID = NetworkManager::GetInstance()->GetMyNetID();
	if (myNetID == 0) myNetID = 1;

	if (NetworkManager::GetInstance()->GetRole() == NetRole::CLIENT)
	{
		SkillChoiceCompletePacket pk{};
		pk.header.type = PacketType::SKILL_CHOICE_COMPLETE;
		pk.header.size = sizeof(SkillChoiceCompletePacket);
		pk.playerNetID = myNetID;
		pk.chosenSkillID = chosenSkillID;
		pk.choiceType = choiceType;

		NetworkManager::GetInstance()->SendPacket(&pk, sizeof(pk));
	}
	else
	{
		NotifySkillChoiceComplete(myNetID, chosenSkillID, choiceType);
	}
}

void InGameManager::NotifySkillChoiceComplete(uint32 netID, uint32 chosenSkillID, uint8 choiceType)
{
	m_clientSkillChoiceMap[netID] = true;
	CheckAndResumeSimulationIfAllChosen();
}

bool InGameManager::IsAllClientsSkillChoiceComplete() const
{
	NetworkManager* net = NetworkManager::GetInstance();
	if (net->GetRole() != NetRole::HOST) return true;

	// Host 본인 확인
	uint32 hostID = net->GetMyNetID();
	if (hostID == 0) hostID = 1;
	auto hostIter = m_clientSkillChoiceMap.find(hostID);
	if (hostIter == m_clientSkillChoiceMap.end() || !hostIter->second)
	{
		return false;
	}

	// 접속된 클라이언트 확인
	const auto& clients = net->GetConnectedClients();
	for (const auto& [netID, info] : clients)
	{
		auto iter = m_clientSkillChoiceMap.find(netID);
		if (iter == m_clientSkillChoiceMap.end() || !iter->second)
		{
			return false;
		}
	}
	return true;
}

void InGameManager::CheckAndResumeSimulationIfAllChosen()
{
	if (NetworkManager::GetInstance()->GetRole() != NetRole::HOST) return;

	if (IsAllClientsSkillChoiceComplete())
	{
		// 모든 인원 선택 완료 -> 게임 재개 신호 브로드캐스트
		SimulationResumeSignalPacket resumePkt{};
		resumePkt.header.type = PacketType::SIMULATION_RESUME_SIGNAL;
		resumePkt.header.size = sizeof(SimulationResumeSignalPacket);
		NetworkManager::GetInstance()->SendPacket(&resumePkt, sizeof(resumePkt));

		m_clientSkillChoiceMap.clear();
		PauseSimulation(false);
	}
}

