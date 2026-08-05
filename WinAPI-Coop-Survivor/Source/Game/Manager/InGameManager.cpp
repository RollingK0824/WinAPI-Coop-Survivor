#include "Engine/Core/pch.h"
#include "InGameManager.h"
#include "Engine/Core/ComponentRegister.h"
#include "Engine/Network/NetworkManager.h"
#include "Engine/Manager/PrefabManager.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Game/Player/Player.h"

static ComponentRegistrar<InGameManager> registrar(EngineKey::CustomComponent::InGameManager.data());

InGameManager::InGameManager(GameObject* owner, TransformComponent* transform):ScriptComponent(owner,transform)
{
}

void InGameManager::Start()
{
	NetworkManager* net = NetworkManager::GetInstance();
	uint32 myNetID = net->GetMyNetID();

	if (myNetID != 0)
	{
		SpawnPlayer(myNetID, true, { 0.0f,0.0f });
	}
}

void InGameManager::Update(float dt)
{
	NetworkManager* net = NetworkManager::GetInstance();
	if (!net->IsConnected())return;
}

GameObject* InGameManager::SpawnPlayer(uint32 netId, bool isLocal, Vector2 spawnPos)
{
	Scene* pScene = gameObject.GetOwnerScene();
	if (!pScene) return nullptr;

	GameObject* pPlayerObj = PrefabManager::GetInstance()->Instantiate("PlayerPrefab", pScene);
	if (!pPlayerObj)
	{
		return nullptr;
	}

	pPlayerObj->transform.SetPosition(spawnPos);

	if (Player* pPlayer = pPlayerObj->GetComponent<Player>())
	{
		pPlayer->SetNetworkInfo(netId, isLocal);
	}

	m_playerObjects[netId] = pPlayerObj;

	return pPlayerObj;
}


